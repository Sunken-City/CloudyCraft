#pragma once

class Vector3;
class Vector2;

class Vector4
{
public:
	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////	
	Vector4();
	Vector4(float initialX, float initialY, float initialZ, float initialW);
	Vector4(const Vector3& baseVector, float initialW);
	Vector4(const Vector2& baseVector, float initialZ, float initialW);
	Vector4(const Vector4& other) = default;

	//FUNCTIONS//////////////////////////////////////////////////////////////////////////
	float CalculateMagnitude() const;
	void Normalize();
	static Vector4 GetNormalized(const Vector4& input);
	static Vector4 GetMidpoint(const Vector4& start, const Vector4& end);
	static float Dot(const Vector4& first, const Vector4& second);

	//OPERATORS//////////////////////////////////////////////////////////////////////////
	Vector4& operator+=(const Vector4& rhs);
	Vector4& operator-=(const Vector4& rhs);
	Vector4& operator*=(const float& scalarConstant);
	Vector4& operator=(const Vector2& rhs);
	Vector4& operator=(const Vector3& rhs);
	//CONSTANTS//////////////////////////////////////////////////////////////////////////
	static const Vector4 ZERO;
	static const Vector4 ONE;
	static const Vector4 UNIT_X;
	static const Vector4 UNIT_Y;
	static const Vector4 UNIT_Z;
	static const Vector4 UNIT_W;

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	#pragma warning(disable:4201) //nonstandard extension used: nameless struct/union
	union {
		float data[4];
		struct {
			float x, y, z, w;
		};
		struct {
			float r, g, b, a;
		};
	};
};

//----------------------------------------------------------------------
inline Vector4 operator+(Vector4 lhs, const Vector4& rhs)
{
	lhs += rhs;
	return lhs;
}

//----------------------------------------------------------------------
inline Vector4 operator-(Vector4 lhs, const Vector4& rhs)
{
	lhs -= rhs;
	return lhs;
}

//----------------------------------------------------------------------
inline Vector4 operator*(Vector4 lhs, const float& scalarConstant)
{
	lhs *= scalarConstant;
	return lhs;
}

//----------------------------------------------------------------------
inline bool operator==(const Vector4& lhs, const Vector4& rhs)
{
	return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z) && (lhs.w == rhs.w);
}

//----------------------------------------------------------------------
inline bool operator!=(const Vector4& lhs, const Vector4& rhs)
{
	return !(lhs == rhs);
}