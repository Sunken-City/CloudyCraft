#pragma once

class Vector2
{
public:
	Vector2();
	Vector2(float initialX, float initialY);
	Vector2(const Vector2& other);
	void SetXY(float newX, float newY);
	float CalculateMagnitude();
	float CalculateTheta();
	void Normalize();

	Vector2& operator+=(const Vector2& rhs);
	Vector2& operator-=(const Vector2& rhs);
	Vector2& operator*=(const Vector2& rhs);
	Vector2& operator*=(const float& scalarConstant);

	static const Vector2 ZERO;
	static const Vector2 ONE;
	static const Vector2 UNIT_X;
	static const Vector2 UNIT_Y;

private:

public:
	float x;
	float y;
};

inline Vector2 operator+(Vector2 lhs, const Vector2& rhs)
{
	lhs += rhs;
	return lhs;
}

inline Vector2 operator-(Vector2 lhs, const Vector2& rhs)
{
	lhs -= rhs;
	return lhs;
}

inline Vector2 operator*(Vector2 lhs, const Vector2& rhs)
{
	lhs *= rhs;
	return lhs;
}

inline Vector2 operator*(Vector2 lhs, const float& scalarConstant)
{
	lhs *= scalarConstant;
	return lhs;
}

inline bool operator==(const Vector2& lhs, const Vector2& rhs)
{ 
	return (lhs.x == rhs.x) && (lhs.y == rhs.y); 
}

inline bool operator!=(const Vector2& lhs, const Vector2& rhs)
{
	return !(lhs == rhs);
}