#include "prism/mugenanimationhandler.h"

#include "prism/collisionhandler.h"
#include "prism/math.h"
#include <prism/log.h>
#include <prism/system.h>

typedef int MugenDuration;

typedef struct {
	int mList;
	int mID;
	Collider mCollider;
} MugenAnimationHandlerHitboxElement;

typedef struct {

	MugenAnimation* mAnimation;
	MugenSpriteFile* mSprites;
	int mStep;

	int mIsFacingRight;
	int mIsFacingDown;

	MugenDuration mOverallTime;
	MugenDuration mStepTime;
	MugenSpriteFileSprite* mSprite;
	int mHasSprite;

	int mHasPassiveHitCB;
	void* mPassiveHitCaller;
	void(*mPassiveHitCB)(void* tCaller, void* tCollisionData);

	int mHasAttackHitCB;
	void* mAttackHitCaller;
	void(*mAttackHitCB)(void* tCaller, void* tCollisionData);

	int mHasAnimationFinishedCallback;
	void* mAnimationFinishedCaller;
	void(*mAnimationFinishedCB)(void* tCaller);

	int mHasPassiveHitboxes;
	int mPassiveCollisionList;
	void* mPassiveCollisionData;

	int mHasAttackHitboxes;
	int mAttackCollisionList;
	void* mAttackCollisionData;

	List mActiveHitboxes;

	Position mPlayerPositionReference;

	double mDrawScale;

	Vector3D mBaseDrawScale;

	Position mOffset;

	int mHasRectangleWidth;
	int mRectangleWidth;

	int mHasRectangleHeight;
	int mRectangleHeight;

	int mHasCameraPositionReference;
	Position* mCameraPositionReference;

	int mHasCameraScaleReference;
	Vector3D* mCameraScaleReference;

	int mHasCameraAngleReference;
	double* mCameraAngleReference;

	int mIsInvisible;

	double mBaseDrawAngle;

	int mHasBasePositionReference;
	Position* mBasePositionReference;

	int mHasScaleReference;
	Vector3D* mScaleReference;

	int mHasAngleReference;
	double* mAngleReference;

	int mHasBlendType;
	BlendType mBlendType;

	int mHasConstraintRectangle;
	GeoRectangle mConstraintRectangle;

	int mIsPaused;
	int mIsLooping;

	double mR;
	double mG;
	double mB;
	double mAlpha;
} MugenAnimationHandlerElement;

static struct {
	IntMap mAnimations;
	int mIsPaused;
} gData;

static void loadMugenAnimationHandler(void* tData) {
	(void)tData;
	gData.mAnimations = new_int_map();
	gData.mIsPaused = 0;
}

static void unloadMugenAnimation(MugenAnimationHandlerElement* e);

static int unloadSingleMugenAnimationCB(void* tCaller, void* tData) {
	(void)tCaller;
	MugenAnimationHandlerElement* e = tData;
	unloadMugenAnimation(e);
	return 1;
}

static void unloadMugenAnimationHandler(void* tData) {
	(void)tData;
	int_map_remove_predicate(&gData.mAnimations, unloadSingleMugenAnimationCB, NULL);
	delete_int_map(&gData.mAnimations);
}

static MugenAnimationStep* getCurrentAnimationStep(MugenAnimationHandlerElement* e) {
	return vector_get(&e->mAnimation->mSteps, min(e->mStep, vector_size(&e->mAnimation->mSteps) - 1));
}


static void passiveAnimationHitCB(void* tCaller, void* tCollisionData) {
	MugenAnimationHandlerElement* e = tCaller;
	if (!e->mHasPassiveHitCB || e->mPassiveHitCB == NULL) return;
	e->mPassiveHitCB(e->mPassiveHitCaller, tCollisionData);
}


static void attackAnimationHitCB(void* tCaller, void* tCollisionData) {
	(void)tCaller;
	(void)tCollisionData;
	MugenAnimationHandlerElement* e = tCaller;
	if (!e->mHasAttackHitCB || e->mAttackHitCB == NULL) return;
	e->mAttackHitCB(e->mAttackHitCaller, tCollisionData);
}

typedef struct {
	MugenAnimationHandlerElement* mElement;
	int mList;
	CollisionCallback mCB;
	void* mCollisionData;
} HitboxAdditionCaller;

static void addSingleHitbox(void* tCaller, void* tData) {
	HitboxAdditionCaller* caller = tCaller;
	CollisionRect* rect = tData;

	MugenAnimationHandlerHitboxElement* e = allocMemory(sizeof(MugenAnimationHandlerHitboxElement));

	CollisionRect scaledRectangle = *rect;
	scaledRectangle = scaleGeoRectangleByFactor(scaledRectangle, caller->mElement->mDrawScale);
	if (!caller->mElement->mIsFacingRight) {
		double xBuffer = scaledRectangle.mTopLeft.x;
		scaledRectangle.mTopLeft.x = -scaledRectangle.mBottomRight.x;
		scaledRectangle.mBottomRight.x = -xBuffer;
	}
	if (!caller->mElement->mIsFacingDown) {
		double yBuffer = scaledRectangle.mTopLeft.y;
		scaledRectangle.mTopLeft.y = -scaledRectangle.mBottomRight.y;
		scaledRectangle.mBottomRight.y = -yBuffer;
	}

	e->mCollider = makeColliderFromRect(scaledRectangle);

	e->mList = caller->mList;
	e->mID = addColliderToCollisionHandler(caller->mList, &caller->mElement->mPlayerPositionReference, e->mCollider, caller->mCB, caller->mElement, caller->mCollisionData);

	list_push_back_owned(&caller->mElement->mActiveHitboxes, e);
}

static void addNewSingleHitboxType(MugenAnimationHandlerElement* e, List* tHitboxes, int tList, CollisionCallback tCB, void* tCollisionData) {
	HitboxAdditionCaller caller;
	caller.mElement = e;
	caller.mList = tList;
	caller.mCB = tCB;
	caller.mCollisionData = tCollisionData;

	list_map(tHitboxes, addSingleHitbox, &caller);
}

static void addNewHitboxes(MugenAnimationHandlerElement* e, MugenAnimationStep* tStep) {
	if (e->mHasPassiveHitboxes) {
		addNewSingleHitboxType(e, &tStep->mPassiveHitboxes, e->mPassiveCollisionList, passiveAnimationHitCB, e->mPassiveCollisionData);
	}
	if (e->mHasAttackHitboxes) {
		addNewSingleHitboxType(e, &tStep->mAttackHitboxes, e->mAttackCollisionList, attackAnimationHitCB, e->mAttackCollisionData);
	}
}

static int removeSingleHitbox(void* tCaller, void* tData) {
	(void)tCaller;
	MugenAnimationHandlerHitboxElement* hitbox = tData;

	removeFromCollisionHandler(hitbox->mList, hitbox->mID);
	destroyCollider(&hitbox->mCollider);
	return 1;
}

static void removeOldHitboxes(MugenAnimationHandlerElement* e) {
	list_remove_predicate(&e->mActiveHitboxes, removeSingleHitbox, NULL);
}

static void updateHitboxes(MugenAnimationHandlerElement* e) {
	removeOldHitboxes(e);
	addNewHitboxes(e, getCurrentAnimationStep(e));
}

static MugenDuration getTimeWhenStepStarts(MugenAnimationHandlerElement* e, int tStep) {
	MugenDuration sum = 0;
	int i;
	for (i = 0; i < tStep; i++) {
		MugenAnimationStep* step = vector_get(&e->mAnimation->mSteps, i);
		sum += step->mDuration;
	}
	return sum;

}

static void unloadMugenAnimation(MugenAnimationHandlerElement* e) {
	removeOldHitboxes(e);
	delete_list(&e->mActiveHitboxes);
}

static int loadNextStepAndReturnIfShouldBeRemoved(MugenAnimationHandlerElement* e) {
	e->mStep++;
	if (e->mStep == vector_size(&e->mAnimation->mSteps)) {
		if (e->mHasAnimationFinishedCallback) {
			e->mAnimationFinishedCB(e->mAnimationFinishedCaller);
		}

		if (e->mIsLooping) {
			e->mStep = e->mAnimation->mLoopStart;
			e->mOverallTime = getTimeWhenStepStarts(e, e->mStep); // TODO: test
		}
		else {
			unloadMugenAnimation(e);
			return 1;
		}
	}

	e->mStepTime = 0;

	MugenAnimationStep* step = getCurrentAnimationStep(e);
	e->mSprite = getMugenSpriteFileTextureReference(e->mSprites, step->mGroupNumber, step->mSpriteNumber);
	e->mHasSprite = e->mSprite != NULL;

	updateHitboxes(e);

	return 0;
}

static void increaseMugenDuration(MugenDuration* tDuration) {
	if (gData.mIsPaused) return;

	(*tDuration)++;
}

static void startNewAnimationWithStartStep(MugenAnimationHandlerElement* e, int tStartStep) {
	e->mOverallTime = getTimeWhenStepStarts(e, tStartStep);

	e->mStep = tStartStep - 1;
	if (loadNextStepAndReturnIfShouldBeRemoved(e)) {
		logError("Unable to start animation, is already over.");
		logErrorInteger(e->mAnimation->mID);
		logErrorInteger(tStartStep);
		abortSystem();
	}
}

int addMugenAnimation(MugenAnimation* tStartAnimation, MugenSpriteFile* tSprites, Position tPosition)
{
	MugenAnimationHandlerElement* e = allocMemory(sizeof(MugenAnimationHandlerElement));
	e->mAnimation = tStartAnimation;
	e->mSprites = tSprites;
	e->mStep = 0;

	e->mIsFacingRight = 1;
	e->mIsFacingDown = 1;

	e->mOverallTime = 0;
	e->mStepTime = 0;
	e->mHasSprite = 0;

	e->mHasPassiveHitCB = 0;
	e->mHasAttackHitCB = 0;
	e->mHasAnimationFinishedCallback = 0;

	e->mHasPassiveHitboxes = 0;
	e->mHasAttackHitboxes = 0;
	e->mActiveHitboxes = new_list();

	e->mPlayerPositionReference = tPosition;
	e->mDrawScale = 1;
	e->mBaseDrawScale = makePosition(1, 1, 1);

	e->mOffset = tPosition;
	e->mHasRectangleWidth = 0;
	e->mHasRectangleHeight = 0;

	e->mHasCameraPositionReference = 0;
	e->mHasCameraScaleReference = 0;
	e->mHasCameraAngleReference = 0;

	e->mIsInvisible = 0;
	e->mBaseDrawAngle = 0;

	e->mHasBasePositionReference = 0;
	e->mHasScaleReference = 0;
	e->mHasAngleReference = 0;
	e->mHasBlendType = 0;
	e->mHasConstraintRectangle = 0;

	e->mIsPaused = 0;
	e->mIsLooping = 1;

	e->mR = e->mG = e->mB = e->mAlpha = 1;


	startNewAnimationWithStartStep(e, 0);

	return int_map_push_back_owned(&gData.mAnimations, e);
}

void removeMugenAnimation(int tID)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	unloadMugenAnimation(e);
	int_map_remove(&gData.mAnimations, tID);
}

int isRegisteredMugenAnimation(int tID) {
	return int_map_contains(&gData.mAnimations, tID);
}

int getMugenAnimationAnimationNumber(int tID)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	return e->mAnimation->mID;
}

int getMugenAnimationAnimationStep(int tID) {
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	return e->mStep;
}

int getMugenAnimationRemainingAnimationTime(int tID)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	int remainingTime = (e->mAnimation->mTotalDuration - e->mOverallTime) - 1;

	remainingTime = max(0, remainingTime); // TODO: fix when reading out

	return remainingTime;
}

void setMugenAnimationFaceDirection(int tID, int tIsFacingRight)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mIsFacingRight = tIsFacingRight;
}

void setMugenAnimationVerticalFaceDirection(int tID, int tIsFacingDown) {
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mIsFacingDown = tIsFacingDown;
}

void setMugenAnimationRectangleWidth(int tID, int tWidth)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mHasRectangleWidth = 1;
	e->mRectangleWidth = tWidth;
}

void setMugenAnimationRectangleHeight(int tID, int tHeight)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mHasRectangleHeight = 1;
	e->mRectangleHeight = tHeight;
}

void setMugenAnimationCameraPositionReference(int tID, Position * tCameraPosition)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mHasCameraPositionReference = 1;
	e->mCameraPositionReference = tCameraPosition;
}

void setMugenAnimationCameraScaleReference(int tID, Position * tCameraScale)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mHasCameraScaleReference = 1;
	e->mCameraScaleReference = tCameraScale;
}

void setMugenAnimationCameraAngleReference(int tID, double * tCameraAngle)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mHasCameraAngleReference = 1;
	e->mCameraAngleReference = tCameraAngle;
}

void setMugenAnimationInvisible(int tID)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mIsInvisible = 1;
}

void setMugenAnimationVisibility(int tID, int tIsVisible) {
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mIsInvisible = !tIsVisible;
}

void setMugenAnimationDrawScale(int tID, Vector3D tScale)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mBaseDrawScale = tScale;
}

void setMugenAnimationDrawSize(int tID, Vector3D tSize)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	double scaleX = tSize.x / e->mSprite->mOriginalTextureSize.x;
	double scaleY = tSize.y / e->mSprite->mOriginalTextureSize.y;
	double scaleZ = tSize.z / 1;
	setMugenAnimationDrawScale(tID, makePosition(scaleX, scaleY, scaleZ));
}

void setMugenAnimationDrawAngle(int tID, double tAngle)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mBaseDrawAngle = tAngle;
}

void setMugenAnimationBaseDrawScale(int tID, double tScale)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mDrawScale = tScale;
}

void setMugenAnimationBasePosition(int tID, Position * tBasePosition)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mHasBasePositionReference = 1;
	e->mBasePositionReference = tBasePosition;
}

void setMugenAnimationScaleReference(int tID, Vector3D * tScale)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mHasScaleReference = 1;
	e->mScaleReference = tScale;
}

void setMugenAnimationAngleReference(int tID, double * tAngle)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mHasAngleReference = 1;
	e->mAngleReference = tAngle;
}

void setMugenAnimationColor(int tID, double tR, double tG, double tB) {
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mR = tR;
	e->mG = tG;
	e->mB = tB;
}

void setMugenAnimationTransparency(int tID, double tOpacity) {
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mAlpha = tOpacity;
}

void setMugenAnimationPosition(int tID, Position tPosition)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mOffset = tPosition;
}

void setMugenAnimationBlendType(int tID, BlendType tBlendType)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mBlendType = tBlendType;
	e->mHasBlendType = 1;
}

void setMugenAnimationSprites(int tID, MugenSpriteFile * tSprites)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mSprites = tSprites;
}

void setMugenAnimationConstraintRectangle(int tID, GeoRectangle tConstraintRectangle)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mConstraintRectangle = tConstraintRectangle;
	e->mHasConstraintRectangle = 1;
}

Position getMugenAnimationPosition(int tID)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	return e->mOffset;
}

int getMugenAnimationIsFacingRight(int tID) {
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	return e->mIsFacingRight;
}

int getMugenAnimationIsFacingDown(int tID) {
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	return e->mIsFacingDown;
}

int getMugenAnimationVisibility(int tID) {
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	return !e->mIsInvisible;
}

Vector3D getMugenAnimationDrawScale(int tID)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	return e->mBaseDrawScale;
}

double getMugenAnimationDrawAngle(int tID) 
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	return e->mBaseDrawAngle;
}

double getMugenAnimationColorRed(int tID)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	return e->mR;
}

double getMugenAnimationColorGreen(int tID)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	return e->mG;
}

double getMugenAnimationColorBlue(int tID)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	return e->mB;
}

double * getMugenAnimationColorRedReference(int tID)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	return &e->mR;
}

double * getMugenAnimationColorGreenReference(int tID)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	return &e->mG;
}

double * getMugenAnimationColorBlueReference(int tID)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	return &e->mB;
}

double * getMugenAnimationTransparencyReference(int tID)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	return &e->mAlpha;
}

double * getMugenAnimationScaleXReference(int tID)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	return &e->mBaseDrawScale.x;
}

double* getMugenAnimationScaleYReference(int tID) {
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	return &e->mBaseDrawScale.y;
}

double * getMugenAnimationBaseScaleReference(int tID)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	return &e->mDrawScale;
}

Position * getMugenAnimationPositionReference(int tID)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	return &e->mOffset;
}

void changeMugenAnimation(int tID, MugenAnimation * tNewAnimation)
{
	changeMugenAnimationWithStartStep(tID, tNewAnimation, 0);
}

void changeMugenAnimationWithStartStep(int tID, MugenAnimation * tNewAnimation, int tStartStep)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mAnimation = tNewAnimation;
	startNewAnimationWithStartStep(e, tStartStep);
}

int isStartingMugenAnimationElementWithID(int tID, int tStepID)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	if (e->mIsPaused) return 0;

	int currentStep = e->mStep + 1;
	return currentStep == tStepID && e->mStepTime == 0;
}



int getTimeFromMugenAnimationElement(int tID, int tStep)
{
	tStep--;

	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	if (tStep >= vector_size(&e->mAnimation->mSteps)) {
		tStep = vector_size(&e->mAnimation->mSteps) - 1; // TODO: think about this
	}

	MugenDuration sum = getTimeWhenStepStarts(e, tStep);

	int offsetFromStep = e->mOverallTime - sum;

	return offsetFromStep;
}

static int getMugenAnimationElementFromTimeOffsetLoop(MugenAnimationHandlerElement* e, int tTime, int tCurrentStep, int dx) {
	int n = vector_size(&e->mAnimation->mSteps);

	int isRunning = 1;
	while (isRunning) {
		MugenAnimationStep* step = vector_get(&e->mAnimation->mSteps, tCurrentStep);
		if (tTime < step->mDuration) return tCurrentStep;
		tTime -= step->mDuration;

		tCurrentStep += dx;
		if (tCurrentStep == n) tCurrentStep = e->mAnimation->mLoopStart;
		if (tCurrentStep < e->mAnimation->mLoopStart) tCurrentStep = n - 1;
	}

	return -1;
}

int getMugenAnimationElementFromTimeOffset(int tID, int tTime)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);

	int ret;
	if (tTime > 0) {
		tTime += e->mStepTime;
		ret = getMugenAnimationElementFromTimeOffsetLoop(e, tTime, e->mStep, 1);
	}
	else {
		MugenAnimationStep* step = vector_get(&e->mAnimation->mSteps, e->mStep);
		tTime *= -1;
		tTime += (step->mDuration - 1) - e->mStepTime;
		ret = getMugenAnimationElementFromTimeOffsetLoop(e, tTime, e->mStep, -1);
	}

	return ret + 1;
}

static int updateSingleMugenAnimation(MugenAnimationHandlerElement* e) {
	if (e->mIsPaused) return 0;

	MugenAnimationStep* step = getCurrentAnimationStep(e);
	increaseMugenDuration(&e->mOverallTime);
	increaseMugenDuration(&e->mStepTime);
	if (step->mDuration == -1) return 0;
	if (e->mStepTime >= step->mDuration) {
		return loadNextStepAndReturnIfShouldBeRemoved(e);
	}

	return 0;
}

static int updateSingleMugenAnimationCB(void* tCaller, void* tData) {
	(void)tCaller;
	MugenAnimationHandlerElement* e = tData;

	return updateSingleMugenAnimation(e);
}

void advanceMugenAnimationOneTick(int tID)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	int shouldBeRemoved = updateSingleMugenAnimation(e); // TODO: think about replacing this
	if (shouldBeRemoved) {
		removeMugenAnimation(tID);
	}
}

void pauseMugenAnimation(int tID)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mIsPaused = 1;
}

void unpauseMugenAnimation(int tID)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mIsPaused = 0;
}

void pauseMugenAnimationHandler() {
	gData.mIsPaused = 1;
}
void unpauseMugenAnimationHandler() {
	gData.mIsPaused = 0;
}

static void updateMugenAnimationHandler(void* tData) {
	(void)tData;

	int_map_remove_predicate(&gData.mAnimations, updateSingleMugenAnimationCB, NULL);
}

void setMugenAnimationCollisionActive(int tID, int tCollisionList, void(*tFunc)(void*, void*), void* tCaller, void* tCollisionData)
{
	setMugenAnimationPassiveCollisionActive(tID, tCollisionList, tFunc, tCaller, tCollisionData);
	setMugenAnimationAttackCollisionActive(tID, tCollisionList, NULL, NULL, NULL);
}

void setMugenAnimationPassiveCollisionActive(int tID, int tCollisionList, void(*tFunc)(void *, void *), void * tCaller, void * tCollisionData)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mHasPassiveHitboxes = 1;
	e->mPassiveCollisionList = tCollisionList;
	e->mPassiveCollisionData = tCollisionData;
	e->mHasPassiveHitCB = 1;
	e->mPassiveHitCB = tFunc;
	e->mPassiveHitCaller = tCaller;
}

void setMugenAnimationAttackCollisionActive(int tID, int tCollisionList, void(*tFunc)(void *, void *), void * tCaller, void * tCollisionData)
{
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mHasAttackHitboxes = 1;
	e->mAttackCollisionList = tCollisionList;
	e->mAttackCollisionData = tCollisionData;
	e->mHasAttackHitCB = 1;
	e->mAttackHitCB = tFunc;
	e->mAttackHitCaller = tCaller;
}

void setMugenAnimationNoLoop(int tID) {
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mIsLooping = 0;
}

void setMugenAnimationCallback(int tID, void(*tFunc)(void*), void* tCaller) {
	MugenAnimationHandlerElement* e = int_map_get(&gData.mAnimations, tID);
	e->mHasAnimationFinishedCallback = 1;
	e->mAnimationFinishedCB = tFunc;
	e->mAnimationFinishedCaller = tCaller;
}

typedef struct {
	MugenAnimationHandlerElement* e;
	MugenAnimationStep* mStep;
	Position mScalePosition;
	Vector3D mScale;
	double mAngle;

	Position mBasePosition;

	Position mCameraCenter;
} DrawSingleMugenAnimationSpriteCaller;

static void drawSingleMugenAnimationSpriteCB(void* tCaller, void* tData) {
	DrawSingleMugenAnimationSpriteCaller* caller = tCaller;
	MugenSpriteFileSubSprite* sprite = tData;

	MugenAnimationHandlerElement* e = caller->e;
	MugenAnimationStep* step = caller->mStep;
	Position p = caller->mBasePosition;
	p = vecAdd(p, makePosition(sprite->mOffset.x, sprite->mOffset.y, 0));

	Rectangle texturePos = makeRectangleFromTexture(sprite->mTexture);
	if (e->mHasRectangleWidth) {
		int newWidth = e->mRectangleWidth - sprite->mOffset.x;
		if (newWidth <= 0) return;
		newWidth = min(newWidth, sprite->mTexture.mTextureSize.x);
		texturePos.bottomRight.x = texturePos.topLeft.x + newWidth;
	}

	if (e->mHasRectangleHeight) {
		int newHeight = e->mRectangleHeight - sprite->mOffset.y;
		if (newHeight <= 0) return;
		newHeight = min(newHeight, sprite->mTexture.mTextureSize.y);
		texturePos.bottomRight.y = texturePos.topLeft.y + newHeight;
	}

	if (e->mHasConstraintRectangle) {

		int minWidth = texturePos.topLeft.x;
		int maxWidth = texturePos.bottomRight.x;

		int leftX = max(minWidth, min(maxWidth, (int)(e->mConstraintRectangle.mTopLeft.x - p.x)));
		int rightX = max(minWidth, min(maxWidth, (int)(e->mConstraintRectangle.mBottomRight.x - p.x)));
		if (leftX == rightX) return;

		p.x += leftX - texturePos.topLeft.x;
		texturePos.topLeft.x = leftX;
		texturePos.bottomRight.x = rightX;

		int minHeight = texturePos.topLeft.y;
		int maxHeight = texturePos.bottomRight.y;

		int upY = max(minHeight, min(maxHeight, (int)(e->mConstraintRectangle.mTopLeft.y - p.y)));
		int downY = max(minHeight, min(maxHeight, (int)(e->mConstraintRectangle.mBottomRight.y - p.y)));
		if (upY == downY) return;

		p.y += upY - texturePos.topLeft.y;
		texturePos.topLeft.y = upY;
		texturePos.bottomRight.y = downY;
	}

	int isFacingRight = e->mIsFacingRight;
	if (step->mIsFlippingHorizontally) isFacingRight ^= 1;

	if (!isFacingRight) {
		Rectangle originalTexturePos = texturePos;
		Position center = e->mPlayerPositionReference;
		double deltaX = center.x - p.x;
		double nRightX = center.x + deltaX;
		double nLeftX = nRightX - abs(originalTexturePos.bottomRight.x - originalTexturePos.topLeft.x);
		p.x = nLeftX;
		texturePos.topLeft.x = originalTexturePos.bottomRight.x;
		texturePos.bottomRight.x = originalTexturePos.topLeft.x;
	}

	int isFacingDown = e->mIsFacingDown;
	if (step->mIsFlippingVertically) isFacingDown ^= 1;

	if (!isFacingDown) {
		Rectangle originalTexturePos = texturePos;
		Position center = e->mPlayerPositionReference;
		double deltaY = center.y - p.y;
		double nRightY = center.y + deltaY;
		double nLeftY = nRightY - abs(originalTexturePos.bottomRight.y - originalTexturePos.topLeft.y);
		p.y = nLeftY;
		texturePos.topLeft.y = originalTexturePos.bottomRight.y;
		texturePos.bottomRight.y = originalTexturePos.topLeft.y;
	}

	p = vecAdd2D(p, step->mDelta); // TODO: check if must be inverted

	if (e->mHasCameraPositionReference) {
		p = vecSub(p, *e->mCameraPositionReference);
	}


	if (step->mIsAddition) {
		setDrawingBlendType(BLEND_TYPE_ADDITION);
	}
	else if (e->mHasBlendType) {
		setDrawingBlendType(e->mBlendType); // TODO: work out step/animation blend type mixing
	}



	setDrawingBaseColorAdvanced(e->mR, e->mG, e->mB);
	setDrawingTransparency(e->mAlpha);
	scaleDrawing3D(caller->mScale, caller->mScalePosition);
	setDrawingRotationZ(caller->mAngle, caller->mScalePosition);

	if (e->mHasCameraScaleReference) {
		scaleDrawing3D(*e->mCameraScaleReference, caller->mCameraCenter);
	}
	if (e->mHasCameraAngleReference) {
		setDrawingRotationZ(*e->mCameraAngleReference, caller->mCameraCenter);
	}

	drawSprite(sprite->mTexture, p, texturePos);
	setDrawingParametersToIdentity();
}

static void drawSingleMugenAnimation(void* tCaller, void* tData) {
	(void)tCaller;
	MugenAnimationHandlerElement* e = tData;

	if (e->mIsInvisible) {
		return;
	}
	if (!e->mHasSprite) {
		return;
	}

	MugenAnimationStep* step = getCurrentAnimationStep(e);

	Vector3D drawScale = e->mBaseDrawScale;
	drawScale = vecScale(drawScale, e->mDrawScale);
	if (e->mHasScaleReference) {
		drawScale.x *= e->mScaleReference->x;
		drawScale.y *= e->mScaleReference->y;
	}
	drawScale.z = 1;


	if (e->mHasBasePositionReference) {
		e->mPlayerPositionReference = *e->mBasePositionReference;
	}
	else {
		e->mPlayerPositionReference = makePosition(0, 0, 0);
	}

	e->mPlayerPositionReference = vecAdd(e->mPlayerPositionReference, e->mOffset);
	Position p = e->mPlayerPositionReference;

	if (e->mHasCameraPositionReference) {
		p = vecSub(p, *e->mCameraPositionReference);
	}

	Position scalePosition = p;
	p = vecSub(p, e->mSprite->mAxisOffset);
	ScreenSize sz = getScreenSize();
	Vector3D cameraCenter;
	if (e->mHasCameraPositionReference) {
		p = vecAdd(p, *e->mCameraPositionReference);
		cameraCenter = vecAdd(*e->mCameraPositionReference, makePosition(sz.x / 2, sz.y / 2, 0));
	}
	else {
		cameraCenter = makePosition(sz.x / 2, sz.y / 2, 0);
	}



	double angle = e->mBaseDrawAngle;
	if (e->mHasAngleReference) {
		angle += *e->mAngleReference;
	}

	DrawSingleMugenAnimationSpriteCaller caller;
	caller.e = e;
	caller.mStep = step;
	caller.mScale = drawScale;
	caller.mScalePosition = scalePosition;
	caller.mBasePosition = p;
	caller.mAngle = angle;
	caller.mCameraCenter = cameraCenter;
	
	list_map(&e->mSprite->mTextures, drawSingleMugenAnimationSpriteCB, &caller);
}

static void drawMugenAnimationHandler(void* tData) {
	(void)tData;
	int_map_map(&gData.mAnimations, drawSingleMugenAnimation, NULL);
}


ActorBlueprint MugenAnimationHandler = {
	.mLoad = loadMugenAnimationHandler,
	.mUnload = unloadMugenAnimationHandler,
	.mUpdate = updateMugenAnimationHandler,
	.mDraw = drawMugenAnimationHandler,
};

ActorBlueprint getMugenAnimationHandlerActorBlueprint()
{
	return MugenAnimationHandler;
}
