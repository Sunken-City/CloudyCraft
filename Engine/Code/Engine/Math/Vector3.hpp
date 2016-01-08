#pragma once

class Vector3Int;
class Vector2;

class Vector3
{
public:
	Vector3();
	Vector3(float initialX, float initialY, float initialZ);
	Vector3(const Vector3& other);
	Vector3(const Vector3Int& other);
	void SetXYZ(float newX, float newY, float newZ);
	float CalculateMagnitude() const;
	void Normalize();
	static Vector3 GetNormalized(const Vector3& input);

	Vector3& operator+=(const Vector3& rhs);
	Vector3& operator-=(const Vector3& rhs);
	Vector3& operator*=(const float& scalarConstant);	
	Vector3& operator=(Vector2 rhs);
	
	static const Vector3 ZERO;
	static const Vector3 ONE;
	static const Vector3 UNIT_X;
	static const Vector3 UNIT_Y;
	static const Vector3 UNIT_Z;
private:

public:
	float x;
	float y;
	float z;
};

inline Vector3 operator+(Vector3 lhs, const Vector3& rhs)
{
	lhs += rhs;
	return lhs;
}

inline Vector3 operator-(Vector3 lhs, const Vector3& rhs)
{
	lhs -= rhs;
	return lhs;
}

inline Vector3 operator*(Vector3 lhs, const float& scalarConstant)
{
	lhs *= scalarConstant;
	return lhs;
}

inline bool operator==(const Vector3& lhs, const Vector3& rhs)
{ 
	return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z); 
}

inline bool operator!=(const Vector3& lhs, const Vector3& rhs)
{
	return !(lhs == rhs);
}