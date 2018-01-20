#include "../include/tari/drawing.h"

#include <stdlib.h>
#include <assert.h>

#include <SDL.h>
#ifdef __EMSCRIPTEN__
#include <SDL/SDL_image.h>
#elif defined _WIN32
#include <SDL_image.h>
#include <SDL_ttf.h>
#endif

#include "tari/log.h"
#include "tari/system.h"
#include "tari/datastructures.h"
#include "tari/memoryhandler.h"
#include "tari/math.h"




typedef struct {

	double a;
	double r;
	double g;
	double b;

	Vector3D mTranslation;
	Vector3D mScale;
	Vector3D mAngle;
	Position mScaleEffectCenter;
	Position mRotationEffectCenter;
	int mIsScaleEffectCenterAbsolute;
	int mIsRotationEffectCenterAbsolute;

	int mFrameStartTime;

	Vector mEffectStack;

	int mIsDisabled;

	BlendType mBlendType;
} DrawingData;

typedef struct {
	TextureData mTexture;
	Position mPos;
	Rectangle mTexturePosition;

	DrawingData mData;

} DrawListSpriteElement;

typedef struct {
	char mText[1024];
	TTF_Font* mFont;
	Position mPos;
	Vector3DI mTextSize;
	Color mColor;
} DrawListTruetypeElement;

typedef enum {
	DRAW_LIST_ELEMENT_TYPE_SPRITE,
	DRAW_LIST_ELEMENT_TYPE_TRUETYPE,

} DrawListElementType;

typedef struct {
	DrawListElementType mType;
	void* mData;
	double mZ;
} DrawListElement;


static Vector gDrawVector;
static DrawingData gData;

extern SDL_Window* gSDLWindow;
SDL_Renderer* gRenderer;

void initDrawing() {
	setDrawingParametersToIdentity();

	if (gSDLWindow == NULL) {
		logError("Unable to create renderer because no window was found.");
		abortSystem();
	}

	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
	gRenderer = SDL_CreateRenderer(gSDLWindow, -1, SDL_RENDERER_ACCELERATED);
	if (gRenderer == NULL) {
		logError("Unable to create renderer.");
		abortSystem();
	}
	SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);

	ScreenSize sz = getScreenSize();
	SDL_RenderSetScale(gRenderer, (float)(640.0 / sz.x), (float)(480.0 / sz.y));

	IMG_Init(IMG_INIT_PNG);
	TTF_Init();

	gDrawVector = new_vector();
	gData.mFrameStartTime = 0;

	gData.mTranslation = makePosition(0, 0, 0);
	gData.mEffectStack = new_vector();
	gData.mIsScaleEffectCenterAbsolute = 1;
	gData.mIsRotationEffectCenterAbsolute = 1;

	gData.mIsDisabled = 0;
}

void drawSprite(TextureData tTexture, Position tPos, Rectangle tTexturePosition) {

	if (gData.mIsDisabled) return;

  debugLog("Draw Sprite");
  debugInteger(tTexture.mTextureSize.x);
  debugInteger(tTexture.mTextureSize.y);

  if (tTexture.mTextureSize.x < 0 || tTexture.mTextureSize.y < 0) {
    logError("Called with invalid textureSize");
    logErrorInteger(tTexture.mTextureSize.x);
    logErrorInteger(tTexture.mTextureSize.y);

    return;
  }


  DrawListSpriteElement* e = allocMemory(sizeof(DrawListSpriteElement));
  e->mTexture = tTexture;
  e->mPos = tPos;
  e->mPos = vecAdd(e->mPos, gData.mTranslation);
  e->mTexturePosition = tTexturePosition;
  e->mData = gData;

  DrawListElement* listElement = allocMemory(sizeof(DrawListElement));
  listElement->mType = DRAW_LIST_ELEMENT_TYPE_SPRITE;
  listElement->mData = e;
  listElement->mZ = tPos.z;
  vector_push_back_owned(&gDrawVector, listElement);
}

static void clearSingleDrawElement(void* tCaller, void* tData) {
	(void)tCaller;
	DrawListElement* e = tData;
	freeMemory(e->mData);
}

static void clearDrawVector() {
	vector_map(&gDrawVector, clearSingleDrawElement, NULL);
	vector_empty(&gDrawVector);
}

void startDrawing() {
	SDL_RenderClear(gRenderer);
	clearDrawVector();
}

static int cmpZ(void* tCaller, void* tData1, void* tData2) {
	(void*)tCaller;
	DrawListElement* e1 = tData1;
	DrawListElement* e2 = tData2;
	
	if (e1->mZ < e2->mZ) return -1;
	if (e1->mZ > e2->mZ) return 1;
	else return 0;
}

static SDL_Rect makeSDLRectFromRectangle(Rectangle tRect) {
	SDL_Rect ret;
	ret.x = min(tRect.topLeft.x, tRect.bottomRight.x);
	ret.y = min(tRect.topLeft.y, tRect.bottomRight.y);

	ret.w = abs(tRect.bottomRight.x - tRect.topLeft.x);
	ret.h = abs(tRect.bottomRight.y - tRect.topLeft.y);

	return ret;
}

static Rectangle makeRectangleFromSDLRect(SDL_Rect tRect) {
	Rectangle ret;
	ret.topLeft.x = tRect.x;
	ret.topLeft.y = tRect.y;

	ret.bottomRight.x = tRect.x + tRect.w;
	ret.bottomRight.y = tRect.y + tRect.h;

	return ret;
}


static SDL_Rect scaleSDLRect(SDL_Rect tRect, Vector3D tScale, Position tCenter) {
	Rectangle rect = makeRectangleFromSDLRect(tRect);

	rect = translateRectangle(rect, vecScale(tCenter, -1));
	rect = scaleRectangle(rect, tScale);
	rect = translateRectangle(rect, tCenter);

	return makeSDLRectFromRectangle(rect);
}

static void drawSortedSprite(DrawListSpriteElement* e) {
	int sizeX = abs(e->mTexturePosition.bottomRight.x - e->mTexturePosition.topLeft.x) + 1;
	int sizeY = abs(e->mTexturePosition.bottomRight.y - e->mTexturePosition.topLeft.y) + 1;

	SDL_Rect srcRect;
	srcRect.x = min(e->mTexturePosition.topLeft.x, e->mTexturePosition.bottomRight.x);
	srcRect.y = min(e->mTexturePosition.topLeft.y, e->mTexturePosition.bottomRight.y);
	srcRect.w = sizeX;
	srcRect.h = sizeY;

	SDL_Rect dstRect;
	dstRect.x = (int)e->mPos.x;
	dstRect.y = (int)e->mPos.y;
	dstRect.w = sizeX;
	dstRect.h = sizeY;

	dstRect = scaleSDLRect(dstRect, e->mData.mScale, e->mData.mScaleEffectCenter);

	Position realEffectPos;
	if (e->mData.mIsScaleEffectCenterAbsolute) {
		realEffectPos = vecAdd(e->mData.mScaleEffectCenter, vecScale(e->mPos, -1));
	}
	else {
		realEffectPos = e->mData.mScaleEffectCenter;
	}


	if (e->mData.mIsRotationEffectCenterAbsolute) {
		realEffectPos = vecAdd(e->mData.mRotationEffectCenter, vecScale(e->mPos, -1));
	}
	else {
		realEffectPos = e->mData.mRotationEffectCenter;
	}
	realEffectPos = vecScale3D(realEffectPos, e->mData.mScale);


	SDL_Point effectCenter;
	effectCenter.x = (int)realEffectPos.x;
	effectCenter.y = (int)realEffectPos.y;

	int flip = 0;
	if (e->mTexturePosition.bottomRight.x < e->mTexturePosition.topLeft.x) flip |= SDL_FLIP_HORIZONTAL;
	if (e->mTexturePosition.bottomRight.y < e->mTexturePosition.topLeft.y) flip |= SDL_FLIP_VERTICAL;

	double angleDegrees = 360 - ((e->mData.mAngle.z * 180) / M_PI);

	Texture texture = e->mTexture.mTexture->mData;
	SDL_SetTextureColorMod(texture->mTexture, (Uint8)(e->mData.r * 0xFF), (Uint8)(e->mData.g * 0xFF), (Uint8)(e->mData.b * 0xFF));
	SDL_SetTextureAlphaMod(texture->mTexture, (Uint8)(e->mData.a * 0xFF));

	if (e->mData.mBlendType == BLEND_TYPE_ADDITION) {
		SDL_SetTextureBlendMode(texture->mTexture, SDL_BLENDMODE_ADD);
	}
	else if(e->mData.mBlendType == BLEND_TYPE_NORMAL){
		SDL_SetTextureBlendMode(texture->mTexture, SDL_BLENDMODE_BLEND);
	} else if (e->mData.mBlendType == BLEND_TYPE_SUBTRACTION) {
		SDL_BlendMode blendMode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_SRC_ALPHA, SDL_BLENDFACTOR_DST_ALPHA, SDL_BLENDOPERATION_REV_SUBTRACT, SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD);
		SDL_SetTextureBlendMode(texture->mTexture, blendMode);
	}
	else {
		logError("Unimplemented blend type");
		logErrorInteger(e->mData.mBlendType);
		abortSystem();
	}

	SDL_RenderCopyEx(gRenderer, texture->mTexture, &srcRect, &dstRect, angleDegrees, &effectCenter, flip);
}

static void drawSortedTruetype(DrawListTruetypeElement* e) {

	double r, g, b;
	getRGBFromColor(e->mColor, &r, &g, &b);
	SDL_Color color;
	color.a = 0xFF;
	color.r = 0xFF;
	color.g = 0xFF;
	color.b = 0xFF;

	SDL_Surface* surface = TTF_RenderText_Blended(e->mFont, e->mText, color); 
	SDL_Texture* texture = SDL_CreateTextureFromSurface(gRenderer, surface); 

	SDL_Rect rect; 
	rect.x = (int)e->mPos.x; 
	rect.y = (int)e->mPos.y;
	rect.w = strlen(e->mText)*e->mTextSize.x; 
	rect.h = e->mTextSize.y; 

	SDL_RenderCopy(gRenderer, texture, NULL, &rect);

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
}

static void drawSorted(void* tCaller, void* tData) {
	(void)tCaller;
	DrawListElement* e = tData;

	if (e->mType == DRAW_LIST_ELEMENT_TYPE_SPRITE) {
		drawSortedSprite(e->mData);
	} else if (e->mType == DRAW_LIST_ELEMENT_TYPE_TRUETYPE) {
		drawSortedTruetype(e->mData);
	}
	else {
		logError("Unrecognized draw type");
		logErrorInteger(e->mType);
		abortSystem();
	}
	
}

void stopDrawing() {
	vector_sort(&gDrawVector,cmpZ, NULL);
	vector_map(&gDrawVector, drawSorted, NULL);
	clearDrawVector();
	SDL_RenderPresent(gRenderer);
}

void waitForScreen() {
	double frameMS = (1.0 / 60) * 1000;
	int frameEndTime = (int)(gData.mFrameStartTime + ceil(frameMS));
	int waitTime = frameEndTime-SDL_GetTicks();

	if (waitTime > 0) {
#ifndef __EMSCRIPTEN__
		SDL_Delay(waitTime);
#endif
	}

	gData.mFrameStartTime = SDL_GetTicks();
}

extern void getRGBFromColor(Color tColor, double* tR, double* tG, double* tB);

static void drawTextInternal() {

}

// TODO: refactor into general drawing code so both have it
static int hasToLinebreak(char* tText, int tCurrent, Position tTopLeft, Position tPos, Vector3D tFontSize, Vector3D tBreakSize, Vector3D tTextBoxSize) {
	
	if (tText[0] == ' ') return 0;
	if (tText[0] == '\n') return 1;
	
	char word[1024];
	int positionsRead;
	sscanf(tText + tCurrent, "%1023s%n", word, &positionsRead);

	Position delta = makePosition(positionsRead * tFontSize.x + (positionsRead-1) * tBreakSize.x, 0, 0);
	Position after = vecAdd(tPos, delta);
	Position bottomRight = vecAdd(tTopLeft, tTextBoxSize);

	return (after.x > bottomRight.x);
}

void drawMultilineText(char* tText, char* tFullText, Position tPosition, Vector3D tFontSize, Color tColor, Vector3D tBreakSize, Vector3D tTextBoxSize) {
	int current = 0;

	setDrawingBaseColor(tColor);

	TextureData fontData = getFontTexture();
	Position pos = tPosition;

	while (tText[current] != '\0') {
		FontCharacterData charData = getFontCharacterData(tText[current]);

		Rectangle tTexturePosition;
		tTexturePosition.topLeft.x = (int)(fontData.mTextureSize.x*charData.mFilePositionX1);
		tTexturePosition.topLeft.y = (int)(fontData.mTextureSize.y*charData.mFilePositionY1);
		tTexturePosition.bottomRight.x = (int)(fontData.mTextureSize.x*charData.mFilePositionX2);
		tTexturePosition.bottomRight.y = (int)(fontData.mTextureSize.y*charData.mFilePositionY2);

		double dx = fabs(tTexturePosition.bottomRight.x - tTexturePosition.topLeft.x);
		double dy = fabs(tTexturePosition.bottomRight.y - tTexturePosition.topLeft.y);
		Vector3D scale = makePosition(1 / dx, 1 / dy, 1);
		scaleDrawing3D(vecScale3D(tFontSize, scale), pos);

		drawSprite(fontData, pos, tTexturePosition);

		pos.x += tFontSize.x + tBreakSize.x;
		current++;

		if (hasToLinebreak(tFullText, current, tPosition, pos, tFontSize, tBreakSize, tTextBoxSize)) {
			pos.x = tPosition.x - (tFontSize.x + tBreakSize.x);
			pos.y += tFontSize.y + tBreakSize.y;
		}
	}

	setDrawingParametersToIdentity();
}

void drawTruetypeText(char * tText, TruetypeFont tFont, Position tPosition, Vector3DI tTextSize, Color tColor)
{
	DrawListTruetypeElement* e = allocMemory(sizeof(DrawListTruetypeElement));
	strcpy(e->mText, tText);
	e->mFont = tFont;
	e->mPos = tPosition;
	e->mTextSize = tTextSize;
	e->mColor = tColor;

	DrawListElement* listElement = allocMemory(sizeof(DrawListElement));
	listElement->mType = DRAW_LIST_ELEMENT_TYPE_TRUETYPE;
	listElement->mData = e;
	listElement->mZ = tPosition.z;
	vector_push_back_owned(&gDrawVector, listElement);
}

void scaleDrawing(double tFactor, Position tScalePosition){
	gData.mScale = makePosition(tFactor,tFactor,1);
	gData.mScaleEffectCenter = tScalePosition;
}

void scaleDrawing3D(Vector3D tFactor, Position tScalePosition){
	gData.mScale = tFactor;
	gData.mScaleEffectCenter = tScalePosition;
}

void setDrawingBaseColor(Color tColor){
	getRGBFromColor(tColor, &gData.r, &gData.g, &gData.b);
}

void setDrawingBaseColorAdvanced(double r, double g, double b) {
	gData.r = r;
	gData.g = g;
	gData.b = b;
}

void setDrawingTransparency(double tAlpha){
	gData.a = tAlpha;
}

void setDrawingRotationZ(double tAngle, Position tPosition){
	gData.mAngle.z = tAngle;
	gData.mRotationEffectCenter = tPosition;
}

void setDrawingParametersToIdentity(){
	setDrawingBaseColor(COLOR_WHITE);
	setDrawingTransparency(1.0);
	scaleDrawing(1, makePosition(0, 0, 0));
	setDrawingRotationZ(0, makePosition(0,0,0));
	setDrawingBlendType(BLEND_TYPE_NORMAL);
}

void setDrawingBlendType(BlendType tBlendType)
{
	gData.mBlendType = tBlendType;
}


typedef struct {
	Vector3D mTranslation;

} TranslationEffect;

typedef struct {
	double mAngle;

} RotationZEffect;

void pushDrawingTranslation(Vector3D tTranslation) {
	tTranslation = vecRotateZ(tTranslation, 2*M_PI-gData.mAngle.z);
	gData.mTranslation = vecAdd(gData.mTranslation, tTranslation);

	TranslationEffect* e = allocMemory(sizeof(TranslationEffect));
	e->mTranslation = tTranslation;
	vector_push_back_owned(&gData.mEffectStack, e);
}
void pushDrawingRotationZ(double tAngle, Vector3D tCenter) {
	gData.mRotationEffectCenter = tCenter;
	gData.mAngle.z += tAngle;

	RotationZEffect* e = allocMemory(sizeof(RotationZEffect));
	e->mAngle = tAngle;
	vector_push_back_owned(&gData.mEffectStack, e);
}

void popDrawingRotationZ() {
	int ind = vector_size(&gData.mEffectStack)-1;
	RotationZEffect* e = vector_get(&gData.mEffectStack, ind);
	
	gData.mAngle.z -= e->mAngle;

	vector_remove(&gData.mEffectStack, ind);
}
void popDrawingTranslation() {
	int ind = vector_size(&gData.mEffectStack) - 1;
	TranslationEffect* e = vector_get(&gData.mEffectStack, ind);

	Vector3D tTranslation = e->mTranslation;
	gData.mTranslation = vecAdd(gData.mTranslation, vecScale(tTranslation,-1));

	vector_remove(&gData.mEffectStack, ind);
}


static uint32_t* getPixelFromSurface(SDL_Surface* tSurface, int x, int y) {
	uint32_t* pixels = tSurface->pixels;
	return &pixels[y*tSurface->w + x];
}

#define PIXEL_BUFFER_SIZE 1000
uint32_t gPixelBuffer[PIXEL_BUFFER_SIZE];

void drawColoredRectangleToTexture(TextureData tDst, Color tColor, Rectangle tTarget) {
	Texture dst = tDst.mTexture->mData;

	double rd, gd, bd;
	getRGBFromColor(tColor, &rd, &gd, &bd);
	uint8_t r = (uint8_t)(rd * 255);
	uint8_t g = (uint8_t)(gd * 255);
	uint8_t b = (uint8_t)(bd * 255);
	
	int w = tTarget.bottomRight.x - tTarget.topLeft.x + 1;
	int h = tTarget.bottomRight.y - tTarget.topLeft.y + 1;
	if (w * h >= PIXEL_BUFFER_SIZE) {
		logError("Over pixel buffer limit.");
		logErrorInteger(w);
		logErrorInteger(h);
		abortSystem();
	}

	uint32_t val = SDL_MapRGB(dst->mSurface->format, r, g, b);
	int i;
	for (i = 0; i < w*h; i++) {
		gPixelBuffer[i] = val;
	}


	SDL_Rect rect;
	rect.x = tTarget.topLeft.x;
	rect.y = tTarget.topLeft.y;
	rect.w = w;
	rect.h = h;
	SDL_UpdateTexture(dst->mTexture, &rect, gPixelBuffer, w*sizeof(uint32_t));
}


void disableDrawing() {
	gData.mIsDisabled = 1;
}

void enableDrawing() {
	gData.mIsDisabled = 0;
}



