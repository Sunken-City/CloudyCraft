#pragma once

#include "Engine/Math/Vector2.hpp"

class AABB2
{
public:
	AABB2();
	AABB2(const Vector2& Mins, const Vector2& Maxs);
	~AABB2();

	Vector2 mins;
	Vector2 maxs;
private:

};
