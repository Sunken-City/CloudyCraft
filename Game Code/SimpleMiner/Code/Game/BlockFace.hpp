#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Renderer/Vertex.hpp"

class BlockFace
{
public:
	BlockFace(const Vector3& first, const Vector3& second, const Vector3& third, const Vector3& fourth);
	BlockFace(const BlockFace& other, const Vector3& offset);
	~BlockFace();

	Vertex_PCT verts[4];
private:

};
