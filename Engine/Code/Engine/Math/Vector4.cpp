#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3Int.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <cmath>

const Vector4 Vector4::ZERO = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
const Vector4 Vector4::ONE = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
const Vector4 Vector4::UNIT_X = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
const Vector4 Vector4::UNIT_Y = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
const Vector4 Vector4::UNIT_Z = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
const Vector4 Vector4::UNIT_W = Vector4(0.0f, 0.0f, 0.0f, 1.0f);

//-----------------------------------------------------------------------------------
Vector4::Vector4()
{
}

//-----------------------------------------------------------------------------------
Vector4::Vector4(float initialX, float initialY, float initialZ, float initialW)
	: x(initialX)
	, y(initialY)
	, z(initialZ)
	, w(initialW)
{
}

//-----------------------------------------------------------------------------------
Vector4::Vector4(const Vector3& baseVector, float initialW)
	: x(baseVector.x)
	, y(baseVector.y)
	, z(baseVector.z)
	, w(initialW)
{

}

//-----------------------------------------------------------------------------------
Vector4::Vector4(const Vector2& baseVector, float initialZ, float initialW)
	: x(baseVector.x)
	, y(baseVector.y)
	, z(initialZ)
	, w(initialW)
{

}

//-----------------------------------------------------------------------------------
float Vector4::CalculateMagnitude() const
{
	return sqrt((x*x) + (y*y) + (z*z) + (w*w));
}

//-----------------------------------------------------------------------------------
void Vector4::Normalize()
{
	float length = CalculateMagnitude();
	if (length == 0.f)
	{
		return;
	}
	x /= length;
	y /= length;
	z /= length;
	w /= length;
}

//-----------------------------------------------------------------------------------
Vector4 Vector4::GetNormalized(const Vector4& input)
{
	float length = input.CalculateMagnitude();
	if (length == 0.f)
	{
		return Vector4::ZERO;
	}
	return Vector4(input.x / length, input.y / length, input.z / length, input.w / length);
}

//-----------------------------------------------------------------------------------
Vector4 Vector4::GetMidpoint(const Vector4& start, const Vector4& end)
{
	Vector4 midpoint;
	midpoint.x = (start.x + end.x) / 2.0f;
	midpoint.y = (start.y + end.y) / 2.0f;
	midpoint.z = (start.z + end.z) / 2.0f;
	midpoint.w = (start.w + end.w) / 2.0f;
	return(midpoint);
}

//-----------------------------------------------------------------------------------
Vector4& Vector4::operator+=(const Vector4& rhs)
{
	this->x += rhs.x;
	this->y += rhs.y;
	this->z += rhs.z;
	this->w += rhs.w;
	return *this;
}

//-----------------------------------------------------------------------------------
Vector4& Vector4::operator-=(const Vector4& rhs)
{
	this->x -= rhs.x;
	this->y -= rhs.y;
	this->z -= rhs.z;
	this->w -= rhs.w;
	return *this;
}

//-----------------------------------------------------------------------------------
Vector4& Vector4::operator*=(const float& scalarConstant)
{
	this->x *= scalarConstant;
	this->y *= scalarConstant;
	this->z *= scalarConstant;
	this->w *= scalarConstant;
	return *this;
}

//-----------------------------------------------------------------------------------
Vector4& Vector4::operator=(const Vector2& rhs)
{
	this->x = rhs.x;
	this->y = rhs.y;
	this->z = 0.0f;
	this->w = 0.0f;
	return *this;
}

//-----------------------------------------------------------------------------------
Vector4& Vector4::operator=(const Vector3& rhs)
{
	this->x = rhs.x;
	this->y = rhs.y;
	this->z = rhs.z;
	this->w = 0.0f;
	return *this;
}

//-----------------------------------------------------------------------------------
float Vector4::Dot(const Vector4& first, const Vector4& second)
{
	return(first.x * second.x) + (first.y * second.y) + (first.z * second.z) + (first.w * second.w);
}