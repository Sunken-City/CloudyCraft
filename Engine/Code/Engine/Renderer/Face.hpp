#pragma once
#include "Engine/Math/Vector3.hpp"

class Face
{
public:
	Face(const Vector3& first, const Vector3& second, const Vector3& third, const Vector3& fourth);
	~Face();

	Vector3 verts[4];
private:

};
