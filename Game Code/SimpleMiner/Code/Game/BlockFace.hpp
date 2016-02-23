#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Renderer/Vertex.hpp"

class BlockFace
{
public:
	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
	BlockFace(const Vector3& first, const Vector3& second, const Vector3& third, const Vector3& fourth);
	BlockFace(const BlockFace& other, const Vector3& posOffset, const Vector2& textureOffset, const RGBA& color);
	~BlockFace();

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	Vertex_PCT verts[4];
};
