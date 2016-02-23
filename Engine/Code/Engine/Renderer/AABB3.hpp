#pragma once
#include "Engine/Math/Vector3.hpp"

class AABB3
{
public:
	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
	AABB3();
	AABB3(const Vector3& Mins, const Vector3& Maxs);
	~AABB3();

	//OPERATORS//////////////////////////////////////////////////////////////////////////
	AABB3& operator+=(const Vector3& rhs);
	AABB3& operator-=(const Vector3& rhs);

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	Vector3 mins;
	Vector3 maxs;
};

//-----------------------------------------------------
inline AABB3 operator+(AABB3 lhs, const Vector3& rhs)
{
	lhs += rhs;
	return lhs;
}

//-----------------------------------------------------
inline AABB3 operator-(AABB3 lhs, const Vector3& rhs)
{
	lhs -= rhs;
	return lhs;
}
