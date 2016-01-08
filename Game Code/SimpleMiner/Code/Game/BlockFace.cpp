#include "Game/BlockFace.hpp"

BlockFace::BlockFace(const Vector3& first, const Vector3& second, const Vector3& third, const Vector3& fourth)
{
	verts[0].pos = first;
	verts[1].pos = second;
	verts[2].pos = third;
	verts[3].pos = fourth;
}

BlockFace::BlockFace(const BlockFace& other, const Vector3& offset)
{
	verts[0] = other.verts[0];
	verts[1] = other.verts[1];
	verts[2] = other.verts[2];
	verts[3] = other.verts[3];
	for (Vertex_PCT vert : verts)
	{
		vert.pos += offset;
	}
}

BlockFace::~BlockFace()
{
}