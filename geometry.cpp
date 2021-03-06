#include "prism/geometry.h"

#define _USE_MATH_DEFINES  
#include <cmath> 
#include <algorithm>

#include "prism/math.h"
#include "prism/log.h"

using namespace std;

Vector2D::Vector2D(double x, double y)
	: x(x)
	, y(y)
{
}

GeoRectangle2D::GeoRectangle2D(double x, double y, double w, double h)
	: mTopLeft(x, y)
	, mBottomRight(x + w, y + h)
{
}

double dot3D(Vector3D p1, Vector3D p2) {
	return p1.x*p2.x + p1.y*p2.y + p1.z*p2.z;
}

double vecLength(Vector3D tVelocity) {
	return fstsqrt(tVelocity.x * tVelocity.x + tVelocity.y * tVelocity.y + tVelocity.z * tVelocity.z);
}

Vector3D vecAdd(Vector3D v1, Vector3D v2) {
	Vector3D ret;
	ret.x = v1.x + v2.x;
	ret.y = v1.y + v2.y;
	ret.z = v1.z + v2.z;
	return ret;
}

Vector3D vecSub(Vector3D v1, Vector3D v2)
{
	return vecAdd(v1, vecScale(v2, -1));
}

Vector3D vecScale(Vector3D v, double tFactor) {
	Vector3D ret;
	ret.x = v.x*tFactor;
	ret.y = v.y*tFactor;
	ret.z = v.z*tFactor;
	return ret;
}

Vector3D vecScale3D(Vector3D v, Vector3D tScale) {
	Vector3D ret;
	ret.x = v.x*tScale.x;
	ret.y = v.y*tScale.y;
	ret.z = v.z*tScale.z;
	return ret;
}

Vector3D vecNormalize(Vector3D tVector) {
	double l = vecLength(tVector);
	if (l == 0) {
		return tVector;
	}
	tVector.x /= l;
	tVector.y /= l;
	tVector.z /= l;
	return tVector;
}

Vector3D vecRotateZ(Vector3D tVector, double tAngle) {
	Vector3D ret;
	ret.x = cos(tAngle)*tVector.x - sin(tAngle)*tVector.y;
	ret.y = sin(tAngle)*tVector.x + cos(tAngle)*tVector.y;
	ret.z = tVector.z;
	return ret;
}

Vector3D vecRotateZAroundCenter(Vector3D tVector, double tAngle, Vector3D tCenter) {
	tVector = vecSub(tVector, tCenter);
	tVector = vecRotateZ(tVector, tAngle);
	tVector = vecAdd(tVector, tCenter);
	return tVector;
}

Vector3D vecScaleToSize(Vector3D v, double tSize)
{
	v = vecNormalize(v);
	v = vecScale(v, tSize);
	return v;
}

Position makePosition(Vector3DI tOtherVector) {
	Position pos;
	pos.x = double(tOtherVector.x);
	pos.y = double(tOtherVector.y);
	pos.z = double(tOtherVector.z);
	return pos;
}

Position makePosition(double x, double y, double z) {
	Position pos;
	pos.x = x;
	pos.y = y;
	pos.z = z;
	return pos;
}

Vector3DI makeVector3DI(int x, int y, int z) {
	Vector3DI v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

Position variatePosition(Position tBase) {
	Position ret;
	ret.x = randfrom(-tBase.x, tBase.x);
	ret.y = randfrom(-tBase.y, tBase.y);
	ret.z = randfrom(-tBase.z, tBase.z);
	return ret;
}

void printPosition(char* tName, Position tPosition) {
	logString(tName);
	logDouble(tPosition.x);
	logDouble(tPosition.y);
	logDouble(tPosition.z);

}

Position getDirection(Position tFrom, Position tTo) {
	Position ret;
	ret.x = tTo.x - tFrom.x;
	ret.y = tTo.y - tFrom.y;
	ret.z = tTo.z - tFrom.z;
	return ret;
}

double getDistance2D(Position tFrom, Position tTo)
{
	tFrom.z = tTo.z = 0;
	return vecLength(vecSub(tTo, tFrom));
}

Line makeLine(Vector3D tStart, Vector3D tEnd) {
	Line ret;
	ret.mP1 = tStart;
	ret.mP2 = tEnd;
	return ret;
}

GeoRectangle makeGeoRectangle(double x, double y, double w, double h)
{
	GeoRectangle ret;
	ret.mTopLeft.x = x;
	ret.mTopLeft.y = y;
	ret.mBottomRight.x = x + w;
	ret.mBottomRight.y = y + h;
	return ret;
}

GeoRectangle makeGeoRectangle3D(double x, double y, double z, double w, double h) {
	GeoRectangle ret;
	ret.mTopLeft.x = x;
	ret.mTopLeft.y = y;
	ret.mTopLeft.z = z;
	ret.mBottomRight.x = x + w;
	ret.mBottomRight.y = y + h;
	ret.mBottomRight.z = z;
	return ret;
}

Vector3DI vecAddI(Vector3DI v1, Vector3DI v2) {
	v1.x += v2.x;
	v1.y += v2.y;
	v1.z += v2.z;
	return v1;
}

Vector3DI vecScaleI(const Vector3DI& v, double tFactor)
{
	return makeVector3DI(int(v.x * tFactor), int(v.y * tFactor), int(v.z * tFactor));
}

int vecEqualsI(Vector3DI v1, Vector3DI v2) {
	return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

int vecEqualsI2D(Vector3DI v1, Vector3DI v2) {
	return v1.x == v2.x && v1.y == v2.y;
}

double vecLength2D(Vector3D v)
{
	return vecLength(makePosition(v.x, v.y, 0));
}

Vector3D vecAdd2D(Vector3D v1, Vector3D v2)
{
	v1.x += v2.x;
	v1.y += v2.y;
	return v1;
}

Vector3D vecSub2D(Vector3D v1, Vector3D v2)
{
	v1.x -= v2.x;
	v1.y -= v2.y;
	return v1;
}

Vector3D vecMin2D(const Vector3D& v1, const Vector3D& v2)
{
	return makePosition(std::min(v1.x, v2.x), std::min(v1.y, v2.y), 0);
}

Vector3D vecMax2D(const Vector3D& v1, const Vector3D& v2)
{
	return makePosition(std::max(v1.x, v2.x), std::max(v1.y, v2.y), 0);
}

Vector3DI vecMinI2D(const Vector3DI& v1, const Vector3DI& v2)
{
	return makeVector3DI(std::min(v1.x, v2.x), std::min(v1.y, v2.y), 0);
}

Vector3DI vecMaxI2D(const Vector3DI& v1, const Vector3DI& v2)
{
	return makeVector3DI(std::max(v1.x, v2.x), std::max(v1.y, v2.y), 0);
}

double getAngleFromDirection(Vector3D tDirection) {
	return -fatan2(tDirection.y, tDirection.x) + M_PI;
}

Vector3D getDirectionFromAngleZ(double tAngle) {
	return makePosition(cos(tAngle), sin(tAngle), 0);
}

double degreesToRadians(double tDegrees) {
	return (tDegrees / 180) * M_PI;
}

double radiansToDegrees(double tRadians) {
	return (tRadians / M_PI) * 180;
}

int checkIntersectLineCircle(Line tLine, Circle tCircle) {

	double r = tCircle.mRadius;

	Position d = getDirection(tLine.mP1, tLine.mP2);
	Position f = getDirection(tCircle.mCenter, tLine.mP1);

	double a = dot3D(d, d);
	double b = 2 * dot3D(f, d);
	double c = dot3D(f, f) - r*r;

	double discriminant = b*b - 4 * a*c;
	if (discriminant < 0)
	{
		return 0;
	}
	else
	{
		// ray didn't totally miss sphere,
		// so there is a solution to
		// the equation.

		discriminant = fstsqrt(discriminant);

		// either solution may be on or off the ray so need to test both
		// t1 is always the smaller value, because BOTH discriminant and
		// a are nonnegative.
		double t1 = (-b - discriminant) / (2 * a);
		double t2 = (-b + discriminant) / (2 * a);

		// 3x HIT cases:
		//          -o->             --|-->  |            |  --|->
		// Impale(t1 hit,t2 hit), Poke(t1 hit,t2>1), ExitWound(t1<0, t2 hit), 

		// 3x MISS cases:
		//       ->  o                     o ->              | -> |
		// FallShort (t1>1,t2>1), Past (t1<0,t2<0), CompletelyInside(t1<0, t2>1)

		if (t1 >= 0 && t1 <= 1)
		{
			// t1 is the intersection, and it's closer than t2
			// (since t1 uses -b - discriminant)
			// Impale, Poke
			return 1;
		}

		// here t1 didn't intersect so we are either started
		// inside the sphere or completely past it
		if (t2 >= 0 && t2 <= 1)
		{
			// ExitWound
			return 1;
		}

		// no intn: FallShort, Past, CompletelyInside
		return 0;
	}
}

int checkPointInCircle(Circle tCirc, Position tPoint) {
	Position d = getDirection(tCirc.mCenter, tPoint);
	double l = vecLength(d);
	return l <= tCirc.mRadius;
}

int checkPointInRectangle(GeoRectangle tRect, Position tPoint) {
	return tPoint.x >= tRect.mTopLeft.x && tPoint.x <= tRect.mBottomRight.x && tPoint.y >= tRect.mTopLeft.y && tPoint.y <= tRect.mBottomRight.y;
}

int checkIntersectCircRect(Circle tCirc, GeoRectangle tRect) {
	Position p1 = tRect.mTopLeft;
	Position p2 = makePosition(tRect.mTopLeft.x, tRect.mBottomRight.y, 0);
	Position p3 = tRect.mBottomRight;
	Position p4 = makePosition(tRect.mBottomRight.x, tRect.mTopLeft.y, 0);

	return (checkPointInRectangle(tRect, tCirc.mCenter) ||
		checkIntersectLineCircle(makeLine(p1, p2), tCirc) ||
		checkIntersectLineCircle(makeLine(p2, p3), tCirc) ||
		checkIntersectLineCircle(makeLine(p3, p4), tCirc) ||
		checkIntersectLineCircle(makeLine(p4, p1), tCirc) ||
		(checkPointInCircle(tCirc, p1) && 
			checkPointInCircle(tCirc, p2) && 
			checkPointInCircle(tCirc, p3)  && 
			checkPointInCircle(tCirc, p4)));

}

Vector3D clampPositionToGeoRectangle(Vector3D v, GeoRectangle tRect)
{
	Vector3D ret = v;
	ret.x = fmax(ret.x, tRect.mTopLeft.x);
	ret.y = fmax(ret.y, tRect.mTopLeft.y);
	ret.x = fmin(ret.x, tRect.mBottomRight.x);
	ret.y = fmin(ret.y, tRect.mBottomRight.y);
	return ret;
}

GeoRectangle scaleGeoRectangleByFactor(GeoRectangle tRect, double tFac)
{
	return scaleGeoRectangleByFactor2D(tRect, makePosition(tFac, tFac, 1));
}

GeoRectangle scaleGeoRectangleByFactor2D(GeoRectangle tRect, Vector3D tFac)
{
	tRect.mTopLeft.x *= tFac.x;
	tRect.mTopLeft.y *= tFac.y;
	tRect.mBottomRight.x *= tFac.x;
	tRect.mBottomRight.y *= tFac.y;
	return tRect;
}

Vector3D interpolatePositionLinear(Position a, Position b, double t)
{
	Vector3D ret;
	ret.x = interpolateLinear(a.x, b.x, t);
	ret.y = interpolateLinear(a.y, b.y, t);
	ret.z = interpolateLinear(a.z, b.z, t);
	return ret;
}

Vector2D operator+(const Vector2D & a, const Vector2D & b)
{
	return Vector2D(a.x + b.x, a.y + b.y);
}

Vector3D operator+(const Vector3D & a, const Vector2D & b)
{
	return makePosition(a.x + b.x, a.y + b.y, a.z);
}

Vector3D operator+(const Vector3D& a, const Vector3D& b) {
	return vecAdd(a, b);
}

Vector3D operator-(const Vector3D& a, const Vector3D& b) {
	return vecSub(a, b);
}

Vector3D operator*(const double & a, const Vector3D & b) {
	return makePosition(b.x * a, b.y * a, b.z * a);
}

Vector3D operator*(const Vector3D& a, const double& b) {
	return makePosition(a.x * b, a.y * b, a.z * b);
}

void operator*=(Vector3D & a, const double & b)
{
	a.x *= b;
	a.y *= b;
	a.z *= b;
}

Vector3D operator/(const Vector3D& a, const double& b) {
	return makePosition(a.x / b, a.y / b, a.z / b);
}

Vector3D operator/(const double& a, const Vector3D& b) {
	return makePosition(a / b.x, a / b.y, a / b.z);
}

Vector3DI operator+(const Vector3DI& a, const Vector3DI& b) {
	return vecAddI(a, b);
}

Vector3D operator+(const Vector3DI& a, const Vector3D& b) {
	return makePosition(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vector3D operator+(const Vector3D& a, const Vector3DI& b) {
	return makePosition(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vector3D operator*(const Vector3D& a, const Vector3D& b) {
	Vector3D ret;
	ret.x = a.x * b.x;
	ret.y = a.y * b.y;
	ret.z = a.z * b.z;
	return ret;
}

int operator==(const Vector3D& a, const Vector3D& b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

int operator!=(const Vector3D& a, const Vector3D& b) {
	return a.x != b.x || a.y != b.y || a.z != b.z;
}

Vector3D& operator+=(Vector3D& a, const Vector3D& b) {
	a = a + b;
	return a;
}

Vector3DI operator-(const Vector3DI& a, const Vector3DI& b) {
	Vector3DI ret = a;
	ret.x -= b.x;
	ret.y -= b.y;
	ret.z -= b.z;
	return ret;
}

Vector3D operator-(const Vector3D& a, const Vector3DI & b)
{
	Vector3D ret = a;
	ret.x -= b.x;
	ret.y -= b.y;
	ret.z -= b.z;
	return ret;
}

Vector3D operator/(const Vector3DI& a, const double& b) {
	return makePosition(a.x / b, a.y / b, a.z / b);
}

Vector3DI operator/(const Vector3DI& a, const int& b) {
	return makeVector3DI(a.x / b, a.y / b, a.z / b);
}

Vector3DI operator*(const Vector3DI& a, const Vector3DI& b) {
	Vector3DI ret;
	ret.x = a.x * b.x;
	ret.y = a.y * b.y;
	ret.z = a.z * b.z;
	return ret;
}

int operator==(const Vector3DI& a, const Vector3DI& b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

int operator!=(const Vector3DI& a, const Vector3DI& b) {
	return a.x != b.x || a.y != b.y || a.z != b.z;
}

GeoRectangle operator*(const GeoRectangle& a, const double& b) {
	GeoRectangle ret;
	ret.mTopLeft = a.mTopLeft*b;
	ret.mBottomRight = a.mBottomRight*b;
	return ret;
}

GeoRectangle operator+(const GeoRectangle& a, const Position& b) {
	GeoRectangle ret;
	ret.mTopLeft = a.mTopLeft + b;
	ret.mBottomRight = a.mBottomRight + b;
	return ret;
}