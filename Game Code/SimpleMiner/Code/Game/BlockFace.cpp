#include "Game/BlockFace.hpp"

//-----------------------------------------------------------------------------------
BlockFace::BlockFace(const Vector3& first, const Vector3& second, const Vector3& third, const Vector3& fourth)
{
	verts[0].pos = first;
	verts[1].pos = second;
	verts[2].pos = third;
	verts[3].pos = fourth;
	verts[0].texCoords = Vector2(0.0f, 0.0f);
	verts[1].texCoords = Vector2(1.0f, 0.0f);
	verts[2].texCoords = Vector2(1.0f, 1.0f);
	verts[3].texCoords = Vector2(0.0f, 1.0f);
}

//-----------------------------------------------------------------------------------
BlockFace::BlockFace(const BlockFace& other, const Vector3& posOffset, const Vector2& textureOffset, const RGBA& color)
{
	for (int i = 0; i < 4; i++)
	{
		verts[i] = other.verts[i];
		verts[i].pos += posOffset;
		//TODO: Redo this part, I don't have a better idea right now.
		verts[i].texCoords.x = verts[i].texCoords.x == 0.0f ? textureOffset.x : textureOffset.x + 0.0625f;
		verts[i].texCoords.y = verts[i].texCoords.y == 0.0f ? textureOffset.y : textureOffset.y - 0.0625f;
		verts[i].color = color;
	}
}

//-----------------------------------------------------------------------------------
BlockFace::~BlockFace()
{
}