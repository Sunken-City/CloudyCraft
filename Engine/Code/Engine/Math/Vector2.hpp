#pragma once

class Vector2Int;

class Vector2
{
public:
	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
	Vector2();
	Vector2(float initialX, float initialY);
	Vector2(const Vector2& other);
	Vector2(const Vector2Int& other);
	void SetXY(float newX, float newY);

	//FUNCTIONS//////////////////////////////////////////////////////////////////////////
	float CalculateMagnitude();
	float CalculateTheta();
	static Vector2 GetMidpoint(const Vector2& start, const Vector2& end);
	void Normalize();

	//OPERATORS//////////////////////////////////////////////////////////////////////////
	Vector2& operator+=(const Vector2& rhs);
	Vector2& operator-=(const Vector2& rhs);
	Vector2& operator*=(const Vector2& rhs);
	Vector2& operator*=(const float& scalarConstant);
	Vector2& operator/=(const float& scalarConstant);

	//CONSTANTS//////////////////////////////////////////////////////////////////////////
	static const Vector2 ZERO;
	static const Vector2 ONE;
	static const Vector2 UNIT_X;
	static const Vector2 UNIT_Y;

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	float x;
	float y;
};

//----------------------------------------------------------------------
inline Vector2 operator+(Vector2 lhs, const Vector2& rhs)
{
	lhs += rhs;
	return lhs;
}

//----------------------------------------------------------------------
inline Vector2 operator-(Vector2 lhs, const Vector2& rhs)
{
	lhs -= rhs;
	return lhs;
}

//----------------------------------------------------------------------
inline Vector2 operator*(Vector2 lhs, const Vector2& rhs)
{
	lhs *= rhs;
	return lhs;
}

//----------------------------------------------------------------------
inline Vector2 operator*(Vector2 lhs, const float& scalarConstant)
{
	lhs *= scalarConstant;
	return lhs;
}

//----------------------------------------------------------------------
inline Vector2 operator/(Vector2 lhs, const float& scalarConstant)
{
	lhs /= scalarConstant;
	return lhs;
}

//----------------------------------------------------------------------
inline bool operator==(const Vector2& lhs, const Vector2& rhs)
{ 
	return (lhs.x == rhs.x) && (lhs.y == rhs.y); 
}

//----------------------------------------------------------------------
inline bool operator!=(const Vector2& lhs, const Vector2& rhs)
{
	return !(lhs == rhs);
}