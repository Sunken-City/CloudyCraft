#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/Vector3Int.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <cmath>

const Vector3 Vector3::ZERO    = Vector3(0.0f, 0.0f, 0.0f);
const Vector3 Vector3::ONE     = Vector3(1.0f, 1.0f, 1.0f);
const Vector3 Vector3::UNIT_X  = Vector3(1.0f, 0.0f, 0.0f);
const Vector3 Vector3::UNIT_Y  = Vector3(0.0f, 1.0f, 0.0f);
const Vector3 Vector3::UNIT_Z  = Vector3(0.0f, 0.0f, 1.0f);
const Vector3 Vector3::FORWARD = Vector3(0.0f, 0.0f, 1.0f);
const Vector3 Vector3::UP      = Vector3(0.0f, 1.0f, 0.0f);
const Vector3 Vector3::RIGHT   = Vector3(1.0f, 0.0f, 0.0f);

//-----------------------------------------------------------------------------------
Vector3::Vector3()
{
}

//-----------------------------------------------------------------------------------
Vector3::Vector3(float initialX, float initialY, float initialZ)
	: x(initialX)
	, y(initialY)
	, z(initialZ)
{
}

//-----------------------------------------------------------------------------------
Vector3::Vector3(const Vector3& other) 
	: x(other.x)
	, y(other.y)
	, z(other.z)
{
}

//-----------------------------------------------------------------------------------
Vector3::Vector3(const Vector3Int& other)
: x(static_cast<float>(other.x))
, y(static_cast<float>(other.y))
, z(static_cast<float>(other.z))
{

}

Vector3::Vector3(float initialValue)
{
	x = initialValue;
	y = initialValue;
	z = initialValue;
}

Vector3::Vector3(const Vector4& other)
{
	x = other.x;
	y = other.y;
	z = other.z;
}

//-----------------------------------------------------------------------------------
void Vector3::SetXYZ(float newX, float newY, float newZ)
{
	x = newX;
	y = newY;
	z = newZ;
}

//-----------------------------------------------------------------------------------
float Vector3::CalculateMagnitude() const
{
	return sqrt((x*x) + (y*y) + (z*z));
}

//-----------------------------------------------------------------------------------
void Vector3::Normalize()
{
	float len = CalculateMagnitude();
	if (len == 0.f)return;
	x /= len;
	y /= len;
	z /= len;
}

//-----------------------------------------------------------------------------------
Vector3 Vector3::Cross(const Vector3& first, const Vector3& second)
{
	Vector3 crossProduct;
	crossProduct.x = (first.y * second.z) - (first.z * second.y);
	crossProduct.y = -((first.x * second.z) - (first.z * second.x));
	crossProduct.z = (first.x * second.y) - (first.y * second.x);
	return crossProduct;
}

//-----------------------------------------------------------------------------------
Vector3 Vector3::GetNormalized(const Vector3& input)
{
	float len = input.CalculateMagnitude();
	if (len == 0.f)
	{
		return Vector3::ZERO;
	}
	return Vector3(input.x / len, input.y / len, input.z / len);
}

//-----------------------------------------------------------------------------------
Vector3 Vector3::GetMidpoint(const Vector3& start, const Vector3& end)
{
	Vector3 midpoint;
	midpoint.x = (start.x + end.x) / 2.0f;
	midpoint.y = (start.y + end.y) / 2.0f;
	midpoint.z = (start.z + end.z) / 2.0f;
	return(midpoint);
}

//-----------------------------------------------------------------------------------
Vector3& Vector3::operator+=(const Vector3& rhs)
{
	this->x += rhs.x;
	this->y += rhs.y;
	this->z += rhs.z;
	return *this;
}

//-----------------------------------------------------------------------------------
Vector3& Vector3::operator-=(const Vector3& rhs)
{
	this->x -= rhs.x;
	this->y -= rhs.y;
	this->z -= rhs.z;
	return *this;
}

//-----------------------------------------------------------------------------------
Vector3& Vector3::operator*=(const float& scalarConstant)
{
	this->x *= scalarConstant;
	this->y *= scalarConstant;
	this->z *= scalarConstant;
	return *this;
}

Vector3& Vector3::operator=(Vector2 rhs)
{
	this->x = rhs.x;
	this->y = rhs.y;
	this->z = 0.0f;
	return *this;
}
