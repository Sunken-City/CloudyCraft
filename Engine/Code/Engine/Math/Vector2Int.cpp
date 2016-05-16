#include "Engine/Math/Vector2Int.hpp"
#include "Engine/Math/Vector2.hpp"
#include <cmath>

const Vector2Int Vector2Int::ZERO = Vector2Int(0, 0);
const Vector2Int Vector2Int::ONE = Vector2Int(1, 1);
const Vector2Int Vector2Int::UNIT_X = Vector2Int(1, 0);
const Vector2Int Vector2Int::UNIT_Y = Vector2Int(0, 1);

//-----------------------------------------------------------------------------------
Vector2Int::Vector2Int()
{
}

//-----------------------------------------------------------------------------------
Vector2Int::Vector2Int(int initialX, int initialY) 
    : x(initialX)
    , y(initialY)
{
}

//-----------------------------------------------------------------------------------
Vector2Int::Vector2Int(const Vector2Int& other) 
    : x(other.x)
    , y(other.y)
{
}

//-----------------------------------------------------------------------------------
Vector2Int::Vector2Int(const Vector2& other)
    : x(static_cast<int>(other.x))
    , y(static_cast<int>(other.y))
{

}

//-----------------------------------------------------------------------------------
void Vector2Int::SetXY(int newX, int NewY)
{
    x = newX;
    y = NewY;
}

//-----------------------------------------------------------------------------------
Vector2Int& Vector2Int::operator+=(const Vector2Int& rhs)
{
    this->x += rhs.x;
    this->y += rhs.y;
    return *this;
}

//-----------------------------------------------------------------------------------
Vector2Int& Vector2Int::operator-=(const Vector2Int& rhs)
{
    this->x -= rhs.x;
    this->y -= rhs.y;
    return *this;
}

//-----------------------------------------------------------------------------------
Vector2Int& Vector2Int::operator*=(const int& scalarConstant)
{
    this->x *= scalarConstant;
    this->y *= scalarConstant;
    return *this;
}

//-----------------------------------------------------------------------------------
Vector2Int Vector2Int::Manhattan(const Vector2Int& position, const Vector2Int& end)
{
    return Vector2Int(static_cast<int>(abs(position.x - end.x)), static_cast<int>(abs(position.y - end.y)));
}

//-----------------------------------------------------------------------------------
float Vector2Int::Magnitude()
{
    return sqrt((x*x) + (y*y));
}
