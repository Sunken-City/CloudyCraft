#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector2Int.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <cmath>

const Vector2 Vector2::ZERO = Vector2(0.0f, 0.0f);
const Vector2 Vector2::ONE = Vector2(1.0f, 1.0f);
const Vector2 Vector2::UNIT_X = Vector2(1.0f, 0.0f);
const Vector2 Vector2::UNIT_Y = Vector2(0.0f, 1.0f);

//-----------------------------------------------------------------------------------
Vector2::Vector2()
{
}

//-----------------------------------------------------------------------------------
Vector2::Vector2(float initialX, float initialY) 
	: x(initialX)
	, y(initialY)
{
}

//-----------------------------------------------------------------------------------
Vector2::Vector2(const Vector2& other) 
	: x(other.x)
	, y(other.y)
{
}

//-----------------------------------------------------------------------------------
Vector2::Vector2(const Vector2Int& other)
	: x(static_cast<float>(other.x))
	, y(static_cast<float>(other.y))
{

}

//-----------------------------------------------------------------------------------
void Vector2::SetXY(float newX, float NewY)
{
	x = newX;
	y = NewY;
}

//-----------------------------------------------------------------------------------
float Vector2::CalculateMagnitude()
{
	return sqrt((x*x) + (y*y));
}

//-----------------------------------------------------------------------------------
void Vector2::Normalize()
{
	float len = CalculateMagnitude();
	if (len == 0.f)return;
	x /= len;
	y /= len;
}

//-----------------------------------------------------------------------------------
float Vector2::CalculateTheta()
{
	return MathUtils::RadiansToDegrees(atan2(y, x));
}

//-----------------------------------------------------------------------------------
Vector2 Vector2::GetMidpoint(const Vector2& start, const Vector2& end)
{
	Vector2 midpoint;
	midpoint.x = (start.x + end.x) / 2.0f;
	midpoint.y = (start.y + end.y) / 2.0f;
	return midpoint;
}

//-----------------------------------------------------------------------------------
Vector2& Vector2::operator+=(const Vector2& rhs)
{
	this->x += rhs.x;
	this->y += rhs.y;
	return *this;
}

//-----------------------------------------------------------------------------------
Vector2& Vector2::operator-=(const Vector2& rhs)
{
	this->x -= rhs.x;
	this->y -= rhs.y;
	return *this;
}

//-----------------------------------------------------------------------------------
Vector2& Vector2::operator*=(const Vector2& rhs)
{

	this->x *= rhs.x;
	this->y *= rhs.y;
	return *this;
}

//-----------------------------------------------------------------------------------
Vector2& Vector2::operator*=(const float& scalarConstant)
{
	this->x *= scalarConstant;
	this->y *= scalarConstant;
	return *this;
}

//-----------------------------------------------------------------------------------
Vector2& Vector2::operator/=(const float& scalarConstant)
{
	this->x /= scalarConstant;
	this->y /= scalarConstant;
	return *this;
}
