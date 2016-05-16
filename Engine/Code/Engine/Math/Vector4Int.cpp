#include "Engine/Math/Vector4Int.hpp"
#include "Engine/Math/Vector4.hpp"
#include <math.h>

const Vector4Int Vector4Int::ZERO = Vector4Int(0, 0, 0, 0);
const Vector4Int Vector4Int::ONE = Vector4Int(1, 1, 1, 1);
const Vector4Int Vector4Int::UNIT_X = Vector4Int(1, 0, 0, 0);
const Vector4Int Vector4Int::UNIT_Y = Vector4Int(0, 1, 0, 0);

//-----------------------------------------------------------------------------------
Vector4Int::Vector4Int()
{
}

//-----------------------------------------------------------------------------------
Vector4Int::Vector4Int(int initialX, int initialY, int initialZ, int initialW)
    : x(initialX)
    , y(initialY)
    , z(initialZ)
    , w(initialW)
{
}

//-----------------------------------------------------------------------------------
Vector4Int::Vector4Int(const Vector4Int& other)
    : x(other.x)
    , y(other.y)
    , z(other.z)
    , w(other.w)
{
}

//-----------------------------------------------------------------------------------
Vector4Int::Vector4Int(const Vector4& other)
    : x(static_cast<int>(other.x))
    , y(static_cast<int>(other.y))
    , z(static_cast<int>(other.z))
    , w(static_cast<int>(other.w))
{

}

//-----------------------------------------------------------------------------------
Vector4Int& Vector4Int::operator+=(const Vector4Int& rhs)
{
    this->x += rhs.x;
    this->y += rhs.y;
    this->z += rhs.z;
    this->w += rhs.w;
    return *this;
}

//-----------------------------------------------------------------------------------
Vector4Int& Vector4Int::operator-=(const Vector4Int& rhs)
{
    this->x -= rhs.x;
    this->y -= rhs.y;
    this->z -= rhs.z;
    this->w -= rhs.w;
    return *this;
}

//-----------------------------------------------------------------------------------
Vector4Int& Vector4Int::operator*=(const int& scalarConstant)
{
    this->x *= scalarConstant;
    this->y *= scalarConstant;
    this->z *= scalarConstant;
    this->w *= scalarConstant;
    return *this;
}

//-----------------------------------------------------------------------------------
Vector4Int Vector4Int::Manhattan(const Vector4Int& position, const Vector4Int& end)
{
    return Vector4Int(static_cast<int>(abs(position.x - end.x))
        , static_cast<int>(abs(position.y - end.y))
        , static_cast<int>(abs(position.z - end.z))
        , static_cast<int>(abs(position.w - end.w)));
}

float Vector4Int::Magnitude()
{
    return sqrt((x*x) + (y*y) + (z*z) + (w*w));
}
