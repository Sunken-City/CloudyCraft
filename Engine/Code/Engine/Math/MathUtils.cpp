#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector2Int.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector3Int.hpp"
#include "Engine/Math/Vector4.hpp"
#include <cmath>
#include <cstdlib>

const float MathUtils::PI = 3.14159265358f;
const float MathUtils::TWO_PI = 3.14159265358f * 2.0f;
const float MathUtils::HALF_PI = 3.14159265358f / 2.0f;

//-----------------------------------------------------------------------------------
float MathUtils::Lerp(float fraction, float initialValue, float endValue)
{
	return initialValue + fraction * (endValue - initialValue);
}

//-----------------------------------------------------------------------------------
Vector2 MathUtils::Lerp(float fraction, const Vector2& initialValue, const Vector2& endValue)
{
	return initialValue + ((endValue - initialValue) * fraction);
}

//-----------------------------------------------------------------------------------
float MathUtils::DegreesToRadians(float degrees)
{
	return degrees * (PI / 180.0f);
}

//-----------------------------------------------------------------------------------
float MathUtils::RadiansToDegrees(float radians)
{
	return radians * (180.0f / PI);
}

//-----------------------------------------------------------------------------------
float MathUtils::CalcDistanceBetweenPoints(const Vector2& pos1, const Vector2& pos2)
{
	float xDist = pos2.x - pos1.x;
	float yDist = pos2.y - pos1.y;
	return sqrt(xDist * xDist + yDist * yDist);
}

//-----------------------------------------------------------------------------------
float MathUtils::CalcDistSquaredBetweenPoints(const Vector2& pos1, const Vector2& pos2)
{
	float xDist = pos2.x - pos1.x;
	float yDist = pos2.y - pos1.y;
	return xDist * xDist + yDist * yDist;
}

//-----------------------------------------------------------------------------------
float MathUtils::CalcDistSquaredBetweenPoints(const Vector3& pos1, const Vector3& pos2)
{
	float xDist = pos2.x - pos1.x;
	float yDist = pos2.y - pos1.y;
	float zDist = pos2.z - pos1.z;
	return xDist * xDist + yDist * yDist + zDist * zDist;
}

//-----------------------------------------------------------------------------------
float MathUtils::CalcDistSquaredBetweenPoints(const Vector2Int& pos1, const Vector2Int& pos2)
{
	float xDist = static_cast<float>(pos2.x) - static_cast<float>(pos1.x);
	float yDist = static_cast<float>(pos2.y) - static_cast<float>(pos1.y);
	return xDist * xDist + yDist * yDist;
}


//-----------------------------------------------------------------------------------
float MathUtils::RangeMap(float inValue, float min1, float max1, float min2, float max2)
{
	//@TODO: Make sure max1 - min1 != 0
	return min2 + ((max2 - min2) * ((inValue - min1) / (max1 - min1)));
}

//-----------------------------------------------------------------------------------
int MathUtils::Clamp(int inputValue, int min, int max)
{
	if (inputValue < min)
	{
		return min;
	}
	else if (inputValue > max)
	{
		return max;
	}
	else
	{
		return inputValue;
	}
}

//-----------------------------------------------------------------------------------
float MathUtils::Clamp(float inputValue, float min, float max)
{
	if (inputValue < min)
	{
		return min;
	}
	else if (inputValue > max)
	{
		return max;
	}
	else
	{
		return inputValue;
	}
}

//-----------------------------------------------------------------------------------
Vector3 MathUtils::Clamp(const Vector3& inputValue, float min, float max)
{
	Vector3 temp;
	temp.x = Clamp(inputValue.x, min, max);
	temp.y = Clamp(inputValue.y, min, max);
	temp.z = Clamp(inputValue.z, min, max);
	return temp;
}

//-----------------------------------------------------------------------------------
float MathUtils::Clamp(float inputValue)
{
	return Clamp(inputValue, 0.0f, 1.0f);
}

//-----------------------------------------------------------------------------------
bool MathUtils::DoDiscsOverlap(const Vector2& center1, float radius1, const Vector2& center2, float radius2)
{
	float distSquared = CalcDistSquaredBetweenPoints(center1, center2);
	float radii = radius1 + radius2;
	return distSquared < (radii * radii);
}

//-----------------------------------------------------------------------------------
bool MathUtils::IsPointInDisk(const Vector2& point, const Vector2& center, float radius)
{
	//A point is a disk with radius 0
	return DoDiscsOverlap(point, 0.f, center, radius);
}

//-----------------------------------------------------------------------------------
float MathUtils::CalcShortestAngularDisplacement(float fromDegrees, float toDegrees)
{
	float angularDisplacement = toDegrees - fromDegrees;
	while (angularDisplacement > 180.f)
	{
		angularDisplacement -= 360.f;
	}
	while (angularDisplacement < -180.f)
	{
		angularDisplacement += 360.f;
	}
	return angularDisplacement;
}

//-----------------------------------------------------------------------------------
int MathUtils::GetRandom(int minimum, int maximum)
{
	return Clamp(rand() % maximum + minimum, minimum, maximum);
}

//-----------------------------------------------------------------------------------
float MathUtils::GetRandom()
{
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

//-----------------------------------------------------------------------------------
float MathUtils::GetRandomFromZeroTo(float maximum)
{
	return static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / maximum));
}

//-----------------------------------------------------------------------------------
float MathUtils::GetRandom(float minimum, float maximum)
{
	return minimum + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (maximum - minimum)));
}

//-----------------------------------------------------------------------------------
float MathUtils::Dot(const Vector2& a, const Vector2& b)
{
	return(a.x * b.x) + (a.y * b.y);
}

//-----------------------------------------------------------------------------------
float MathUtils::Dot(const Vector3& a, const Vector3& b)
{
	return(a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

//-----------------------------------------------------------------------------------
float MathUtils::Dot(const Vector4& a, const Vector4& b)
{
	return(a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}

//-----------------------------------------------------------------------------------
float MathUtils::CosDegrees(float input)
{
	return cos(DegreesToRadians(input));
}

//-----------------------------------------------------------------------------------
float MathUtils::SinDegrees(float input)
{
	return sin(DegreesToRadians(input));
}

//-----------------------------------------------------------------------------------
float MathUtils::SmoothStep(float x)
{
	float xSquared = x * x;
	float xCubed = xSquared * x;
	return ((3.0f * xSquared) - (2.0f * xCubed));
}

//-----------------------------------------------------------------------------------
Vector3 MathUtils::RemoveDirectionalComponent(const Vector3& original, const Vector3& directionToStripOut)
{
	float lengthAlongNormal = Dot(original, directionToStripOut);
	Vector3 partsAlongNormal = Vector3::GetNormalized(original) * lengthAlongNormal;
	return (original + partsAlongNormal);
}

//-----------------------------------------------------------------------------------
void SetBit(uchar& bitFlags, uchar bitMask)
{
	bitFlags |= bitMask;
}

//-----------------------------------------------------------------------------------
bool IsBitSet(uchar bitFlags, uchar bitMask)
{
	return(bitFlags & bitMask) != 0;
}

//-----------------------------------------------------------------------------------
void ClearBit(uchar& bitFlags, uchar bitMask)
{
	bitFlags &= ~bitMask;
}

