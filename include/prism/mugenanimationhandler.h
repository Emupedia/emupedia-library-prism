#pragma once 

#include <list>

#include "actorhandler.h"
#include "geometry.h"
#include "mugenspritefilereader.h"
#include "mugenanimationreader.h"
#include "collision.h"

typedef int MugenDuration;
struct CollisionListData;
struct CollisionListElement;

typedef struct {
	CollisionListData* mList;
	CollisionListElement* mElement;
	Collider mCollider;
} MugenAnimationHandlerHitboxElement;

struct MugenAnimationHandlerElement {

	int mID;
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
	CollisionListData* mPassiveCollisionList;
	void* mPassiveCollisionData;

	int mHasAttackHitboxes;
	CollisionListData* mAttackCollisionList;
	void* mAttackCollisionData;

	std::list<MugenAnimationHandlerHitboxElement> mActiveHitboxes;

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

	int mHasCameraEffectPositionReference;
	Position* mCameraEffectPositionReference;

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
	int mHasLooped;

	double mR;
	double mG;
	double mB;
	double mAlpha;

	int mIsCollisionDebugActive;
};

MugenAnimationHandlerElement* addMugenAnimation(MugenAnimation* tStartAnimation, MugenSpriteFile* tSprites, Position tPosition);
void removeMugenAnimation(MugenAnimationHandlerElement* tElement);
int isRegisteredMugenAnimation(MugenAnimationHandlerElement* tElement);

void setMugenAnimationBaseDrawScale(MugenAnimationHandlerElement* tElement, double tScale);
void setMugenAnimationBasePosition(MugenAnimationHandlerElement* tElement, Position* tPosition);
void setMugenAnimationScaleReference(MugenAnimationHandlerElement* tElement, Vector3D* tScale);
void setMugenAnimationAngleReference(MugenAnimationHandlerElement* tElement, double* tAngle);

void setMugenAnimationCollisionActive(MugenAnimationHandlerElement* tElement, CollisionListData* tCollisionList, void(*tFunc)(void*, void*), void* tCaller, void* tCollisionData);
void setMugenAnimationPassiveCollisionActive(MugenAnimationHandlerElement* tElement, CollisionListData* tCollisionList, void(*tFunc)(void*, void*), void* tCaller, void* tCollisionData);
void setMugenAnimationAttackCollisionActive(MugenAnimationHandlerElement* tElement, CollisionListData* tCollisionList, void(*tFunc)(void*, void*), void* tCaller, void* tCollisionData);

void setMugenAnimationNoLoop(MugenAnimationHandlerElement* tElement);
void setMugenAnimationCallback(MugenAnimationHandlerElement* tElement, void(*tFunc)(void*), void* tCaller);

int getMugenAnimationAnimationNumber(MugenAnimationHandlerElement* tElement);
int getMugenAnimationAnimationStep(MugenAnimationHandlerElement* tElement);
int getMugenAnimationAnimationStepAmount(MugenAnimationHandlerElement* tElement);
int getMugenAnimationAnimationStepDuration(MugenAnimationHandlerElement* tElement);
int getMugenAnimationRemainingAnimationTime(MugenAnimationHandlerElement* tElement);
int hasMugenAnimationLooped(MugenAnimationHandlerElement* tElement);
int getMugenAnimationTime(MugenAnimationHandlerElement* tElement);
int getMugenAnimationDuration(MugenAnimationHandlerElement* tElement);
Vector3DI getMugenAnimationSprite(MugenAnimationHandlerElement* tElement);
void setMugenAnimationFaceDirection(MugenAnimationHandlerElement* tElement, int tIsFacingRight);
void setMugenAnimationVerticalFaceDirection(MugenAnimationHandlerElement* tElement, int tIsFacingDown);
void setMugenAnimationRectangleWidth(MugenAnimationHandlerElement* tElement, int tWidth);
void setMugenAnimationRectangleHeight(MugenAnimationHandlerElement* tElement, int tHeight);
void setMugenAnimationCameraPositionReference(MugenAnimationHandlerElement* tElement, Position* tCameraPosition);
void setMugenAnimationCameraScaleReference(MugenAnimationHandlerElement* tElement, Position* tCameraScale);
void setMugenAnimationCameraAngleReference(MugenAnimationHandlerElement* tElement, double* tCameraAngle);
void setMugenAnimationCameraEffectPositionReference(MugenAnimationHandlerElement* tElement, Position* tCameraEffectPosition);

void setMugenAnimationInvisible(MugenAnimationHandlerElement* tElement);
void setMugenAnimationVisibility(MugenAnimationHandlerElement* tElement, int tIsVisible);
void setMugenAnimationDrawScale(MugenAnimationHandlerElement* tElement, Vector3D tScale);
void setMugenAnimationDrawSize(MugenAnimationHandlerElement* tElement, Vector3D tSize);
void setMugenAnimationDrawAngle(MugenAnimationHandlerElement* tElement, double tAngle);
void setMugenAnimationColor(MugenAnimationHandlerElement* tElement, double tR, double tG, double tB);
void setMugenAnimationTransparency(MugenAnimationHandlerElement* tElement, double tOpacity);
void setMugenAnimationPosition(MugenAnimationHandlerElement* tElement, Position tPosition);
void setMugenAnimationBlendType(MugenAnimationHandlerElement* tElement, BlendType tBlendType);
void setMugenAnimationSprites(MugenAnimationHandlerElement* tElement, MugenSpriteFile* tSprites);
void setMugenAnimationConstraintRectangle(MugenAnimationHandlerElement* tElement, GeoRectangle tConstraintRectangle);

Position getMugenAnimationPosition(MugenAnimationHandlerElement* tElement);
int getMugenAnimationIsFacingRight(MugenAnimationHandlerElement* tElement);
int getMugenAnimationIsFacingDown(MugenAnimationHandlerElement* tElement);
int getMugenAnimationVisibility(MugenAnimationHandlerElement* tElement);
Vector3D getMugenAnimationDrawScale(MugenAnimationHandlerElement* tElement);

double getMugenAnimationDrawAngle(MugenAnimationHandlerElement* tElement);
double getMugenAnimationColorRed(MugenAnimationHandlerElement* tElement);
double getMugenAnimationColorGreen(MugenAnimationHandlerElement* tElement);
double getMugenAnimationColorBlue(MugenAnimationHandlerElement* tElement);

double* getMugenAnimationColorRedReference(MugenAnimationHandlerElement* tElement);
double* getMugenAnimationColorGreenReference(MugenAnimationHandlerElement* tElement);
double* getMugenAnimationColorBlueReference(MugenAnimationHandlerElement* tElement);
double* getMugenAnimationTransparencyReference(MugenAnimationHandlerElement* tElement);
double* getMugenAnimationScaleXReference(MugenAnimationHandlerElement* tElement);
double* getMugenAnimationScaleYReference(MugenAnimationHandlerElement* tElement);
double* getMugenAnimationBaseScaleReference(MugenAnimationHandlerElement* tElement);
Position* getMugenAnimationPositionReference(MugenAnimationHandlerElement* tElement);

void setMugenAnimationAnimationStepDuration(MugenAnimationHandlerElement* tElement, int tDuration);

void changeMugenAnimation(MugenAnimationHandlerElement* tElement, MugenAnimation* tNewAnimation);
void changeMugenAnimationWithStartStep(MugenAnimationHandlerElement* tElement, MugenAnimation* tNewAnimation, int tStartStep);

int isStartingMugenAnimationElementWithID(MugenAnimationHandlerElement* tElement, int tStepID);
int getTimeFromMugenAnimationElement(MugenAnimationHandlerElement* tElement, int tStep);
int getMugenAnimationElementFromTimeOffset(MugenAnimationHandlerElement* tElement, int tTime);
int isMugenAnimationTimeOffsetInAnimation(MugenAnimationHandlerElement* tElement, int tTime);
int getMugenAnimationTimeWhenStepStarts(MugenAnimationHandlerElement* tElement, int tStep);

void advanceMugenAnimationOneTick(MugenAnimationHandlerElement* tElement);

int hasMugenAnimationChanged(MugenAnimationHandlerElement* tElement, const MugenAnimationHandlerElement& tCompareData);
MugenAnimationHandlerElement saveMugenAnimation(MugenAnimationHandlerElement* tElement);
void restoreMugenAnimation(MugenAnimationHandlerElement* tElement, const MugenAnimationHandlerElement& tRestorationData);

void setMugenAnimationCollisionDebug(MugenAnimationHandlerElement* tElement, int tIsActive);

void pauseMugenAnimation(MugenAnimationHandlerElement* tElement);
void unpauseMugenAnimation(MugenAnimationHandlerElement* tElement);

void pauseMugenAnimationHandler();
void unpauseMugenAnimationHandler();

ActorBlueprint getMugenAnimationHandler();