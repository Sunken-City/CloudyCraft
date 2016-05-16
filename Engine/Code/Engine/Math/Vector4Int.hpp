#pragma once

class Vector4;

class Vector4Int
{
public:
    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    Vector4Int();
    Vector4Int(int initialX, int initialY, int initialZ, int initialW);
    Vector4Int(const Vector4& other);
    Vector4Int(const Vector4Int& other);

    //OPERATORS//////////////////////////////////////////////////////////////////////////
    Vector4Int& operator+=(const Vector4Int& rhs);
    Vector4Int& operator-=(const Vector4Int& rhs);
    Vector4Int& operator*=(const int& scalarConstant);
    static Vector4Int Manhattan(const Vector4Int& position, const Vector4Int& end);
    float Magnitude();
    //CONSTANTS//////////////////////////////////////////////////////////////////////////
    static const Vector4Int ZERO;
    static const Vector4Int ONE;
    static const Vector4Int UNIT_X;
    static const Vector4Int UNIT_Y;

    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    int x;
    int y;
    int z;
    int w;
};

//----------------------------------------------------------------------
inline Vector4Int operator+(Vector4Int lhs, const Vector4Int& rhs)
{
    lhs += rhs;
    return lhs;
}

//----------------------------------------------------------------------
inline Vector4Int operator-(Vector4Int lhs, const Vector4Int& rhs)
{
    lhs -= rhs;
    return lhs;
}

//----------------------------------------------------------------------
inline Vector4Int operator*(Vector4Int lhs, const int& scalarConstant)
{
    lhs *= scalarConstant;
    return lhs;
}

//----------------------------------------------------------------------
inline bool operator==(const Vector4Int& lhs, const Vector4Int& rhs)
{
    return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z) && (lhs.w == rhs.w);
}

//----------------------------------------------------------------------
inline bool operator!=(const Vector4Int& lhs, const Vector4Int& rhs)
{
    return !(lhs == rhs);
}

//-----------------------------------------------------------------------------------
inline Vector4Int operator-(const Vector4Int &rhs)
{
    return rhs * -1;
}