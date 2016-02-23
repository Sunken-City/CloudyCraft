#pragma once

class Vector2;
class Vector2Int;
class Vector3;
class Vector3Int;
class Vector4;

#define BIT(x) (1<<(x))
typedef unsigned char uchar;

class MathUtils
{
public:
	//DISTANCE CALCULATION//////////////////////////////////////////////////////////////////////////
	static float CalcDistanceBetweenPoints(const Vector2& pos1, const Vector2& pos2);
	static float CalcDistSquaredBetweenPoints(const Vector2& pos1, const Vector2& pos2);
	static float CalcDistSquaredBetweenPoints(const Vector2Int& pos1, const Vector2Int& pos2);
	static float CalcDistSquaredBetweenPoints(const Vector3& pos1, const Vector3& pos2);

	//CLAMPING AND MAPPING//////////////////////////////////////////////////////////////////////////
	static float RangeMap(float inValue, float min1, float max1, float min2, float max2);
	static float Clamp(float inputValue);
	static float Clamp(float inputValue, float min, float max);
	static int Clamp(int inputValue, int min, int max);
	static Vector3 Clamp(const Vector3& inputValue, float min, float max);
	static Vector3 RemoveDirectionalComponent(const Vector3& original, const Vector3& directionToStripOut);

	//DISK MATH//////////////////////////////////////////////////////////////////////////
	static bool DoDiscsOverlap(const Vector2& center1, float radius1, const Vector2& center2, float radius2);
	static bool IsPointInDisk(const Vector2& point, const Vector2&  center, float radius);
	static float CalcShortestAngularDisplacement(float fromDegrees, float toDegrees);

	//RANDOM//////////////////////////////////////////////////////////////////////////
	static int GetRandom(int minimum, int maximum);
	static float GetRandom();
	static float GetRandom(float minimum, float maximum);
	static float GetRandomFromZeroTo(float maximum);

	//TRIGONOMETRY//////////////////////////////////////////////////////////////////////////
	static float Dot(const Vector2& a, const Vector2& b);
	static float Dot(const Vector3& a, const Vector3& b);
	static float Dot(const Vector4& a, const Vector4& b);
	static float CosDegrees(float input);
	static float SinDegrees(float input);
	static float DegreesToRadians(float degrees);
	static float RadiansToDegrees(float radians);

	//INTERPOLATION//////////////////////////////////////////////////////////////////////////
	static float SmoothStep(float x);
	static float Lerp(float fraction, float initialValue, float endValue);
	static Vector2 Lerp(float fraction, const Vector2& initialValue, const Vector2& endValue);


	//CONSTANTS//////////////////////////////////////////////////////////////////////////
	static const float PI;
	static const float TWO_PI;
	static const float HALF_PI;
};

//Global functions. I'm planning on moving everything out of the MathUtils class as soon as I have time to refactor all the code the change will break.

//BIT MANIPULATION//////////////////////////////////////////////////////////////////////////
void SetBit(uchar& bitFlags, uchar bitMask);
bool IsBitSet(uchar bitFlags, uchar bitMask);
void ClearBit(uchar& bitFlags, uchar bitMask);