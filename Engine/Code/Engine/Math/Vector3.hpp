#pragma once

class Vector3Int;
class Vector2;
class Vector4;

class Vector3
{
public:
	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////	
	Vector3();
	Vector3(float initialValue);
	Vector3(float initialX, float initialY, float initialZ);
	Vector3(const Vector3& other);
	Vector3(const Vector4& other);
	Vector3(const Vector3Int& other);
	void SetXYZ(float newX, float newY, float newZ);

	//FUNCTIONS//////////////////////////////////////////////////////////////////////////
	float CalculateMagnitude() const;
	void Normalize();
	static Vector3 Cross(const Vector3& first, const Vector3& second);
	static Vector3 GetNormalized(const Vector3& input);
	static Vector3 GetMidpoint(const Vector3& start, const Vector3& end);

	//OPERATORS//////////////////////////////////////////////////////////////////////////
	Vector3& operator+=(const Vector3& rhs);
	Vector3& operator-=(const Vector3& rhs);
	Vector3& operator*=(const float& scalarConstant);	
	Vector3& operator=(Vector2 rhs);
	
	//CONSTANTS//////////////////////////////////////////////////////////////////////////
	static const Vector3 ZERO;
	static const Vector3 ONE;
	static const Vector3 UNIT_X;
	static const Vector3 UNIT_Y;
	static const Vector3 UNIT_Z;
	static const Vector3 FORWARD;
	static const Vector3 UP;
	static const Vector3 RIGHT;

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	float x;
	float y;
	float z;
};

//----------------------------------------------------------------------
inline Vector3 operator+(Vector3 lhs, const Vector3& rhs)
{
	lhs += rhs;
	return lhs;
}

//----------------------------------------------------------------------
inline Vector3 operator-(Vector3 lhs, const Vector3& rhs)
{
	lhs -= rhs;
	return lhs;
}

//----------------------------------------------------------------------
inline Vector3 operator*(Vector3 lhs, const float& scalarConstant)
{
	lhs *= scalarConstant;
	return lhs;
}

//----------------------------------------------------------------------
inline Vector3 operator*(const float& scalarConstant, Vector3 rhs)
{
	rhs *= scalarConstant;
	return rhs;
}


//----------------------------------------------------------------------
inline bool operator==(const Vector3& lhs, const Vector3& rhs)
{ 
	return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z); 
}

//----------------------------------------------------------------------
inline bool operator!=(const Vector3& lhs, const Vector3& rhs)
{
	return !(lhs == rhs);
}

//----------------------------------------------------------------------
inline Vector3 operator-(const Vector3 &rhs)
{
	return rhs * -1.0f;
}