#include "prism/collisionanimation.h"
#include "prism/collisionhandler.h"
#include "prism/memoryhandler.h"
#include "prism/log.h"
#include "prism/system.h"


CollisionAnimation makeEmptyCollisionAnimation() {
	CollisionAnimation ret;

	ret.mFrames = new_vector();
	ret.mAnimation = createEmptyAnimation();
	return ret;
}


static void destroyCollisionAnimationFrame(void* tCaller, void* tData) {
	(void) tCaller;
	Collider* col = (Collider*)tData;
	destroyCollider(col);
}

void destroyCollisionAnimation(CollisionAnimation* tAnimation){
	vector_map(&tAnimation->mFrames, destroyCollisionAnimationFrame, NULL);
	delete_vector(&tAnimation->mFrames);
}

AnimationResult updateCollisionAnimation(CollisionAnimation* tAnimation) {
	return animateWithoutLoop(&tAnimation->mAnimation);

}

void resetCollisionAnimation(CollisionAnimation* tAnimation) {
	resetAnimation(&tAnimation->mAnimation);
}	


Collider getCollisionAnimationCollider(CollisionAnimation* tAnimation) {
	Collider* col = (Collider*)vector_get(&tAnimation->mFrames, tAnimation->mAnimation.mFrame);
	return *col;
}

static struct {

	List mList;

} gPrismCollisionAnimationData;

typedef struct {

	CollisionListData* mCollisionHandlerList;
	CollisionListElement* mCollisionHandlerElement;
	CollisionAnimation mAnimation;

	Position mCenter;

	int mIsInverted;
	Position mPosition;
	Position* mBasePosition;
	

} HandledCollisionAnimation;

static void loadCollisionAnimationHandler(void*) {
	setProfilingSectionMarkerCurrentFunction();
	gPrismCollisionAnimationData.mList = new_list();
}

static void cleanupHandledCollisionAnimation(HandledCollisionAnimation* tAnimation) {
	removeFromCollisionHandler(tAnimation->mCollisionHandlerElement);
}

static int removeAllCollisionAnimations(void* tCaller, void* tData) {
	(void) tCaller;
	HandledCollisionAnimation* collision = (HandledCollisionAnimation*)tData;

	cleanupHandledCollisionAnimation(collision);
	return 1;
}

static void unloadCollisionAnimationHandler(void*) {
	setProfilingSectionMarkerCurrentFunction();
	list_remove_predicate(&gPrismCollisionAnimationData.mList, removeAllCollisionAnimations, NULL);
}


static void invertPositionRect(HandledCollisionAnimation* tData, CollisionRect tRect) {
	double nposX = tRect.mBottomRight.x;

	double dx = nposX - tData->mCenter.x;

	double cx = (tData->mCenter.x - dx) - tRect.mTopLeft.x;

	tData->mPosition.x += cx;
}

static void invertPosition(HandledCollisionAnimation* tData) {
	Collider col = getCollisionAnimationCollider(&tData->mAnimation);
	
	if(col.mType == COLLISION_RECT) {
		CollisionRect* rect = (CollisionRect*)col;
		invertPositionRect(tData, *rect);
	} else {
		logError("Unsupported collision type");
		logErrorInteger(col.mType);
		recoverFromError();
	}
}

static int updateSingleCollisionAnimation(void* tCaller, void* tData) {
	(void) tCaller;
	HandledCollisionAnimation* collision = (HandledCollisionAnimation*)tData;

	Frame before = collision->mAnimation.mAnimation.mFrame;
	AnimationResult res = updateCollisionAnimation(&collision->mAnimation);
	if(res == ANIMATION_OVER) {
		cleanupHandledCollisionAnimation(collision);
		return 1;
	}

	Frame after = collision->mAnimation.mAnimation.mFrame;

	if(before != after)  {
		Collider* nCol = (Collider*)vector_get(&collision->mAnimation.mFrames, after);
		setColliderBasePosition(nCol, &collision->mPosition);
		updateColliderForCollisionHandler(collision->mCollisionHandlerElement, *nCol);
	}

	collision->mPosition = *collision->mBasePosition;
	if(collision->mIsInverted) {
		invertPosition(collision);
	}
	
	return 0;

}

static void updateCollisionAnimationHandler(void*) {
	setProfilingSectionMarkerCurrentFunction();
	list_remove_predicate(&gPrismCollisionAnimationData.mList, updateSingleCollisionAnimation, NULL);
}

ActorBlueprint getCollisionAnimationHandler()
{
	return makeActorBlueprint(loadCollisionAnimationHandler, unloadCollisionAnimationHandler, updateCollisionAnimationHandler);
}

int addHandledCollisionAnimation(CollisionListData* tListID, Position* tBasePosition, CollisionAnimation tAnimation, CollisionCallback tCB, void* tCaller, void* tCollisionData) {
	HandledCollisionAnimation* e = (HandledCollisionAnimation*)allocMemory(sizeof(HandledCollisionAnimation));

	e->mAnimation = tAnimation;
	e->mCollisionHandlerList = tListID;
	e->mCenter = makePosition(0,0,0);
	e->mIsInverted = 0;
	e->mPosition = *tBasePosition;
	e->mBasePosition = tBasePosition;
	
	Collider* firstCollider = (Collider*)vector_get(&tAnimation.mFrames, 0);

	e->mCollisionHandlerElement = addColliderToCollisionHandler(tListID, &e->mPosition, *firstCollider, tCB, tCaller, tCollisionData);

	return list_push_front_owned(&gPrismCollisionAnimationData.mList, e);
}

void removeHandledCollisionAnimation(int tID) {
	HandledCollisionAnimation* e = (HandledCollisionAnimation*)list_get(&gPrismCollisionAnimationData.mList, tID);
	cleanupHandledCollisionAnimation(e);

	list_remove(&gPrismCollisionAnimationData.mList, tID);
}

void invertCollisionAnimationVertical(int tID) {
	HandledCollisionAnimation* e = (HandledCollisionAnimation*)list_get(&gPrismCollisionAnimationData.mList, tID);

	e->mIsInverted ^= 1;

	if(e->mIsInverted) {
		invertPosition(e);
	}
}

void setCollisionAnimationCenter(int tID, Position tCenter) {
	HandledCollisionAnimation* e = (HandledCollisionAnimation*)list_get(&gPrismCollisionAnimationData.mList, tID);

	e->mCenter = tCenter;
}

