#include "Engine/Renderer/Face.hpp"

//-----------------------------------------------------------------------------------
Face::Face(const Vector3& first, const Vector3& second, const Vector3& third, const Vector3& fourth)
{
	verts[0] = first;
	verts[1] = second;
	verts[2] = third;
	verts[3] = fourth;
}

//-----------------------------------------------------------------------------------
Face::~Face()
{
}