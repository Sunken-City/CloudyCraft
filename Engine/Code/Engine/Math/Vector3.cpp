#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3Int.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <math.h>

const Vector3 Vector3::ZERO = Vector3(0.0f, 0.0f, 0.0f);
const Vector3 Vector3::ONE = Vector3(1.0f, 1.0f, 1.0f);
const Vector3 Vector3::UNIT_X = Vector3(1.0f, 0.0f, 0.0f);
const Vector3 Vector3::UNIT_Y = Vector3(0.0f, 1.0f, 0.0f);
const Vector3 Vector3::UNIT_Z = Vector3(0.0f, 0.0f, 1.0f);

Vector3::Vector3()
{
}

Vector3::Vector3(float initialX, float initialY, float initialZ)
	: x(initialX)
	, y(initialY)
	, z(initialZ)
{
}

Vector3::Vector3(const Vector3& other) 
	: x(other.x)
	, y(other.y)
	, z(other.z)
{
}

Vector3::Vector3(const Vector3Int& other)
: x(static_cast<float>(other.x))
, y(static_cast<float>(other.y))
, z(static_cast<float>(other.z))
{

}

void Vector3::SetXYZ(float newX, float newY, float newZ)
{
	x = newX;
	y = newY;
	z = newZ;
}

float Vector3::CalculateMagnitude() const
{
	return sqrt((x*x) + (y*y) + (z*z));
}

void Vector3::Normalize()
{
	float len = CalculateMagnitude();
	if (len == 0.f)return;
	x /= len;
	y /= len;
	z /= len;
}

Vector3 Vector3::GetNormalized(const Vector3& input)
{
	float len = input.CalculateMagnitude();
	if (len == 0.f)
	{
		return Vector3::ZERO;
	}
	return Vector3(input.x / len, input.y / len, input.z / len);
}

Vector3& Vector3::operator+=(const Vector3& rhs)
{
	this->x += rhs.x;
	this->y += rhs.y;
	this->z += rhs.z;
	return *this;
}

Vector3& Vector3::operator-=(const Vector3& rhs)
{
	this->x -= rhs.x;
	this->y -= rhs.y;
	this->z -= rhs.z;
	return *this;
}

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
