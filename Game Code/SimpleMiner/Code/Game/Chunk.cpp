#include "Chunk.hpp"
#include "Engine/Math/Vector3Int.hpp"
#include "Engine/Math/Noise.hpp"
#include "Engine/Renderer/Vertex.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/BlockDefinition.h"
#include "Game/World.hpp"
#include <map>

Chunk::Chunk(const ChunkCoords& chunkCoords, World* world)
: m_chunkPosition(chunkCoords)
, m_bottomLeftCorner(WorldPosition(static_cast<float>((m_chunkPosition.x * BLOCKS_WIDE_X)), static_cast<float>((m_chunkPosition.y * BLOCKS_WIDE_Y)), 0.0f))
, m_eastChunk(nullptr)
, m_westChunk(nullptr)
, m_southChunk(nullptr)
, m_northChunk(nullptr)
, m_isDirty(true)
, m_world(world)
, m_vboID(0)
, m_numVerts(0)
{
	memset(m_blocks, 0, sizeof(m_blocks[0]) * BLOCKS_PER_CHUNK);
	GenerateChunk();
}


Chunk::Chunk(const ChunkCoords& chunkCoords, std::vector<unsigned char>& data, World* world)
: m_chunkPosition(chunkCoords)
, m_bottomLeftCorner(WorldPosition(static_cast<float>((m_chunkPosition.x * BLOCKS_WIDE_X)), static_cast<float>((m_chunkPosition.y * BLOCKS_WIDE_Y)), 0.0f))
, m_eastChunk(nullptr)
, m_westChunk(nullptr)
, m_southChunk(nullptr)
, m_northChunk(nullptr)
, m_isDirty(true)
, m_world(world)
, m_vboID(0)
, m_numVerts(0)
{
	memset(m_blocks, 0, sizeof(m_blocks[0]) * BLOCKS_PER_CHUNK);
	LoadChunkFromData(data);
}

Chunk::~Chunk()
{
	TheRenderer::instance->DeleteBuffers(m_vboID);
}

void Chunk::Update(float deltaTime)
{
	UNUSED(deltaTime);
}

void Chunk::UpdateVAIfDirty()
{
	if (m_isDirty)
	{
		GenerateVertexArray();
		m_isDirty = false;
	}
}

void Chunk::Render() const
{
	TheRenderer::instance->DrawVBO_PCT(m_vboID, m_numVerts, TheRenderer::QUADS, TheGame::instance->m_blockSheet->GetTexture());
}

void Chunk::GenerateChunk()
{
	StartTiming(g_generationProfiling);
	const int MIN_HEIGHT = BLOCKS_TALL_Z / 3;
	const int MAX_HEIGHT = (BLOCKS_TALL_Z * 3) / 4;
	const int SEA_LEVEL = BLOCKS_TALL_Z / 2;
	const float GRID_SIZE = 100.0f;
	const int NUM_OCTAVES = 5;
	const float PERSISTENCE = 0.30f;
	std::map<Vector2Int, float> heights;

	Vector2Int chunkPosInWorld = Vector2Int(m_chunkPosition.x * BLOCKS_WIDE_X, m_chunkPosition.y * BLOCKS_WIDE_X);
	for (int x = chunkPosInWorld.x; x < (chunkPosInWorld.x + BLOCKS_WIDE_X); x++)
	{
		for (int y = chunkPosInWorld.y; y < (chunkPosInWorld.y + BLOCKS_WIDE_Y); y++)
		{
			Vector2 currentColumn = Vector2(static_cast<float>(x), static_cast<float>(y));
			float delta = ComputePerlinNoise2D(currentColumn, GRID_SIZE, NUM_OCTAVES, PERSISTENCE);
			heights[Vector2Int(x, y)] = round(MathUtils::RangeMap(delta, -1.0f, 1.0f, static_cast<float>(MIN_HEIGHT), static_cast<float>(MAX_HEIGHT)));
		}
	}

	for (int i = 0; i < BLOCKS_PER_CHUNK; i++)
	{
		WorldCoords globalCoords = GetWorldCoordsForBlockIndex(i);
		Vector2Int currentColumn = Vector2Int(globalCoords.x, globalCoords.y);
		float height = heights[currentColumn];
		float z = static_cast<float>(globalCoords.z);
		if (z > height)
		{
			if (z <= SEA_LEVEL)
				m_blocks[i].m_type = BlockType::WATER;
			else
				m_blocks[i].m_type = BlockType::AIR;
		}
		else if (z < SEA_LEVEL)
		{
			m_blocks[i].m_type = BlockType::STONE;
		}
		else if (z == SEA_LEVEL)
		{
			m_blocks[i].m_type = BlockType::SAND;
		}
		else if (z < height)
		{
			m_blocks[i].m_type = BlockType::DIRT;
		}
		else if (z == height)
		{
			m_blocks[i].m_type = BlockType::GRASS;
		}
	}
	EndTiming(g_generationProfiling);
}

void Chunk::CalculateSkyLighting()
{
	//Sky Pass 1
	for (int x = 0; x < (BLOCKS_WIDE_X); x++)
	{
		for (int y = 0; y < BLOCKS_WIDE_Y; y++)
		{
			BlockInfo info = GetBlockInfoFromLocalCoords(LocalCoords(x, y, BLOCKS_TALL_Z - 1));
			while (info.m_index != BlockInfo::INVALID_INDEX)
			{
				Block* currentBlock = info.GetBlock();
				if (currentBlock->GetDefinition()->m_opacity != RGBA(0x00000000))
				{
					info = BlockInfo::INVALID_BLOCK;
				}
				else
				{
					currentBlock->SetSky(true);
					currentBlock->SetRedLightValue(RGBA::GetRed(World::SKY_LIGHT));
					currentBlock->SetGreenLightValue(RGBA::GetGreen(World::SKY_LIGHT));
					currentBlock->SetBlueLightValue(RGBA::GetBlue(World::SKY_LIGHT));
					info = info.GetBelow();
				}
			}
		}
	}
	//Sky Pass 2
	for (int x = 0; x < (BLOCKS_WIDE_X); x++)
	{
		for (int y = 0; y < BLOCKS_WIDE_Y; y++)
		{
			BlockInfo info = GetBlockInfoFromLocalCoords(LocalCoords(x, y, BLOCKS_TALL_Z - 1));
			while (info.m_index != BlockInfo::INVALID_INDEX)
			{
				Block* currentBlock = info.GetBlock();
				uchar blockType = currentBlock->m_type;
				if (BlockDefinition::GetDefinition(blockType)->m_isOpaque)
				{
					info = BlockInfo::INVALID_BLOCK;
				}
				else
				{
					BlockInfo eastBlock = info.GetEast();
					if (eastBlock.m_index != BlockInfo::INVALID_INDEX && !eastBlock.GetBlock()->IsSky())
					{
						BlockInfo::SetDirtyFlagAndAddToDirtyList(eastBlock);
					}
					BlockInfo westBlock = info.GetWest();
					if (westBlock.m_index != BlockInfo::INVALID_INDEX && !westBlock.GetBlock()->IsSky())
					{
						BlockInfo::SetDirtyFlagAndAddToDirtyList(westBlock);
					}
					BlockInfo northBlock = info.GetNorth();
					if (northBlock.m_index != BlockInfo::INVALID_INDEX && !northBlock.GetBlock()->IsSky())
					{
						BlockInfo::SetDirtyFlagAndAddToDirtyList(northBlock);
					}
					BlockInfo southBlock = info.GetSouth();
					if (southBlock.m_index != BlockInfo::INVALID_INDEX && !southBlock.GetBlock()->IsSky())
					{
						BlockInfo::SetDirtyFlagAndAddToDirtyList(southBlock);
					}
					info = info.GetBelow();
				}
			}
		}
	}

	for (int i = 0; i < BLOCKS_PER_CHUNK; i++)
	{
		Block* currentBlock = GetBlock(i);
		BlockDefinition* definition = BlockDefinition::GetDefinition(currentBlock->m_type);
		if (definition->m_illumination > 0)
		{
			SetDirtyFlagAndAddToDirtyList(i);
		}
	}
	FlagEdgesAsDirtyLighting();
}

WorldCoords Chunk::GetWorldCoordsForBlockIndex(LocalIndex index) const
{
	LocalCoords coords = GetLocalCoordsFromBlockIndex(index);
	return WorldCoords(coords.x + m_bottomLeftCorner.x, coords.y + m_bottomLeftCorner.y, coords.z);
}

LocalIndex Chunk::GetBlockIndexFromWorldCoords(const WorldCoords& coords) const
{
	LocalCoords localCoords = GetLocalCoordsFromWorldCoords(coords);
	return GetBlockIndexFromLocalCoords(localCoords);
}

LocalCoords Chunk::GetLocalCoordsFromWorldCoords(const WorldCoords &coords) const
{
	LocalCoords localCoords = LocalCoords(coords.x - m_bottomLeftCorner.x, coords.y - m_bottomLeftCorner.y, coords.z);
	if (localCoords.z > BLOCKS_TALL_Z)
	{
		localCoords.z = BLOCKS_TALL_Z;
	}
	else if (localCoords.z < 0)
	{
		localCoords.z = 0;
	}
	return localCoords;
}

LocalCoords Chunk::GetLocalCoordsFromWorldPosition(const WorldPosition &pos) const
{
	return GetLocalCoordsFromWorldCoords(WorldCoords(static_cast<int>(floor(pos.x)), static_cast<int>(floor(pos.y)), static_cast<int>(floor(pos.z))));
}

LocalIndex Chunk::GetBlockIndexFromWorldPosition(const WorldPosition& pos) const
{
	return GetBlockIndexFromWorldCoords(WorldCoords(static_cast<int>(floor(pos.x)), static_cast<int>(floor(pos.y)), static_cast<int>(floor(pos.z))));
}

bool Chunk::IsInFrustum(const Vector3& cameraXYZ, const WorldPosition& playerPosition) const
{
	const Vector3 chunkMins = Vector3(static_cast<float>(m_chunkPosition.x * Chunk::BLOCKS_WIDE_X), static_cast<float>(m_chunkPosition.y * Chunk::BLOCKS_WIDE_Y), 0.0f);
	const Vector3 chunkOffsetX = Vector3::UNIT_X * Chunk::BLOCKS_WIDE_X;
	const Vector3 chunkOffsetY = Vector3::UNIT_Y * Chunk::BLOCKS_WIDE_Y;
	const Vector3 chunkOffsetZ = Vector3::UNIT_Z * Chunk::BLOCKS_TALL_Z;
	const Vector3 chunkMaxs = Vector3(static_cast<float>(m_chunkPosition.x * Chunk::BLOCKS_WIDE_X), static_cast<float>(m_chunkPosition.y * Chunk::BLOCKS_WIDE_Y), static_cast<float>(Chunk::BLOCKS_TALL_Z));
	if (MathUtils::Dot(cameraXYZ, chunkMins - playerPosition) > 0.0f)
	{
		return true;
	}
	else if (MathUtils::Dot(cameraXYZ, chunkMins + chunkOffsetX - playerPosition) > 0.0f)
	{
		return true;
	}
	else if (MathUtils::Dot(cameraXYZ, chunkMins + chunkOffsetY - playerPosition) > 0.0f)
	{
		return true;
	}
	else if (MathUtils::Dot(cameraXYZ, chunkMins + chunkOffsetX + chunkOffsetY - playerPosition) > 0.0f)
	{
		return true;
	}
	else if (MathUtils::Dot(cameraXYZ, chunkMins + chunkOffsetZ - playerPosition) > 0.0f)
	{
		return true;
	}
	else if (MathUtils::Dot(cameraXYZ, chunkMins + chunkOffsetX + chunkOffsetZ - playerPosition) > 0.0f)
	{
		return true;
	}
	else if (MathUtils::Dot(cameraXYZ, chunkMins + chunkOffsetY + chunkOffsetZ - playerPosition) > 0.0f)
	{
		return true;
	}
	else if (MathUtils::Dot(cameraXYZ, chunkMaxs - playerPosition) > 0.0f)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Chunk::GenerateVertexArray()
{
	StartTiming(g_vaBuildingProfiling);
	if (m_vboID == 0)
	{
		m_vboID = TheRenderer::instance->GenerateBufferID();
	}
	std::vector<Vertex_PCT> m_vertexArray;
	m_vertexArray.reserve(1000);
	const float blockSize = 1.0f;
	for (int i = 0; i < BLOCKS_PER_CHUNK; i++)
	{
		Block currentBlock = m_blocks[i];
		if (!currentBlock.GetDefinition()->m_isOpaque)
		{
			continue;
		}

		WorldPosition coords = GetWorldPositionForBlockIndex(i);
		Vertex_PCT vertex;
		vertex.color = RGBA(0x000000FF);
		BlockDefinition* currentDefinition = BlockDefinition::GetDefinition(currentBlock.m_type);
		AABB2 topTex = currentDefinition->GetTopIndex();
		AABB2 sideTex = currentDefinition->GetSideIndex();
		AABB2 bottomTex = currentDefinition->GetBottomIndex();

		Block* belowBlock = GetBelow(i);
		if (belowBlock && !BlockDefinition::GetDefinition(belowBlock->m_type)->m_isOpaque)
		{
			vertex.color = RGBA(belowBlock->GetDampedLightValue(0x33));
			vertex.texCoords = bottomTex.mins;
			vertex.pos = Vector3(coords.x, coords.y, coords.z);
			m_vertexArray.push_back(vertex);
			vertex.texCoords = Vector2(bottomTex.maxs.x, bottomTex.mins.y);
			vertex.pos = Vector3(coords.x, coords.y + blockSize, coords.z);
			m_vertexArray.push_back(vertex);
			vertex.texCoords = bottomTex.maxs;
			vertex.pos = Vector3(coords.x + blockSize, coords.y + blockSize, coords.z);
			m_vertexArray.push_back(vertex);
			vertex.texCoords = Vector2(bottomTex.mins.x, bottomTex.maxs.y);
			vertex.pos = Vector3(coords.x + blockSize, coords.y, coords.z);
			m_vertexArray.push_back(vertex);
		}

		Block* aboveBlock = GetAbove(i);
		if (aboveBlock && !BlockDefinition::GetDefinition(aboveBlock->m_type)->m_isOpaque)
		{
			vertex.color = RGBA(aboveBlock->GetLightValue());
			vertex.texCoords = Vector2(topTex.mins.x, topTex.mins.y);
			vertex.pos = Vector3(coords.x, coords.y, coords.z + blockSize);
			m_vertexArray.push_back(vertex);
			vertex.texCoords = Vector2(topTex.maxs.x, topTex.mins.y);
			vertex.pos = Vector3(coords.x + blockSize, coords.y, coords.z + blockSize);
			m_vertexArray.push_back(vertex);
			vertex.texCoords = Vector2(topTex.maxs.x, topTex.maxs.y);
			vertex.pos = Vector3(coords.x + blockSize, coords.y + blockSize, coords.z + blockSize);
			m_vertexArray.push_back(vertex);
			vertex.texCoords = Vector2(topTex.mins.x, topTex.maxs.y);
			vertex.pos = Vector3(coords.x, coords.y + blockSize, coords.z + blockSize);
			m_vertexArray.push_back(vertex);
		}

		Block* westBlock = GetWest(i);
		if (westBlock && !BlockDefinition::GetDefinition(westBlock->m_type)->m_isOpaque)
		{
			vertex.color = RGBA(westBlock->GetDampedLightValue(0x22));
			vertex.texCoords = Vector2(sideTex.mins.x, sideTex.mins.y);
			vertex.pos = Vector3(coords.x, coords.y + blockSize, coords.z);
			m_vertexArray.push_back(vertex);
			vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.mins.y);
			vertex.pos = Vector3(coords.x, coords.y, coords.z);
			m_vertexArray.push_back(vertex);
			vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.maxs.y);
			vertex.pos = Vector3(coords.x, coords.y, coords.z + blockSize);
			m_vertexArray.push_back(vertex);
			vertex.texCoords = Vector2(sideTex.mins.x, sideTex.maxs.y);
			vertex.pos = Vector3(coords.x, coords.y + blockSize, coords.z + blockSize);
			m_vertexArray.push_back(vertex);
		}

		Block* eastBlock = GetEast(i);
		if (eastBlock && !BlockDefinition::GetDefinition(eastBlock->m_type)->m_isOpaque)
		{
			vertex.color = RGBA(eastBlock->GetDampedLightValue(0x22));
			vertex.texCoords = Vector2(sideTex.mins.x, sideTex.mins.y);
			vertex.pos = Vector3(coords.x + blockSize, coords.y, coords.z);
			m_vertexArray.push_back(vertex);
			vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.mins.y);
			vertex.pos = Vector3(coords.x + blockSize, coords.y + blockSize, coords.z);
			m_vertexArray.push_back(vertex);
			vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.maxs.y);
			vertex.pos = Vector3(coords.x + blockSize, coords.y + blockSize, coords.z + blockSize);
			m_vertexArray.push_back(vertex);
			vertex.texCoords = Vector2(sideTex.mins.x, sideTex.maxs.y);
			vertex.pos = Vector3(coords.x + blockSize, coords.y, coords.z + blockSize);
			m_vertexArray.push_back(vertex);
		}

		Block* southBlock = GetSouth(i);
		if (southBlock && !BlockDefinition::GetDefinition(southBlock->m_type)->m_isOpaque)
		{
			vertex.color = RGBA(southBlock->GetDampedLightValue(0x11));
			vertex.texCoords = Vector2(sideTex.mins.x, sideTex.mins.y);
			vertex.pos = Vector3(coords.x, coords.y, coords.z);
			m_vertexArray.push_back(vertex);
			vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.mins.y);
			vertex.pos = Vector3(coords.x + blockSize, coords.y, coords.z);
			m_vertexArray.push_back(vertex);
			vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.maxs.y);
			vertex.pos = Vector3(coords.x + blockSize, coords.y, coords.z + blockSize);
			m_vertexArray.push_back(vertex);
			vertex.texCoords = Vector2(sideTex.mins.x, sideTex.maxs.y);
			vertex.pos = Vector3(coords.x, coords.y, coords.z + blockSize);
			m_vertexArray.push_back(vertex);
		}

		Block* northBlock = GetNorth(i);
		if (northBlock && !BlockDefinition::GetDefinition(northBlock->m_type)->m_isOpaque)
		{
			vertex.color = RGBA(northBlock->GetDampedLightValue(0x11));
			vertex.texCoords = Vector2(sideTex.mins.x, sideTex.mins.y);
			vertex.pos = Vector3(coords.x + blockSize, coords.y + blockSize, coords.z);
			m_vertexArray.push_back(vertex);
			vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.mins.y);
			vertex.pos = Vector3(coords.x, coords.y + blockSize, coords.z);
			m_vertexArray.push_back(vertex);
			vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.maxs.y);
			vertex.pos = Vector3(coords.x, coords.y + blockSize, coords.z + blockSize);
			m_vertexArray.push_back(vertex);
			vertex.texCoords = Vector2(sideTex.mins.x, sideTex.maxs.y);
			vertex.pos = Vector3(coords.x + blockSize, coords.y + blockSize, coords.z + blockSize);
			m_vertexArray.push_back(vertex);
		}		
	}

// 	StartTiming(g_temporaryProfiling);
// 	for (int i = 0; i < BLOCKS_PER_CHUNK; i++)
// 	{
// 		WorldPosition coords = GetWorldPositionForBlockIndex(i);
// 	}
// 	EndTiming(g_temporaryProfiling);

	//Opaque drawing
	for (int i = 0; i < BLOCKS_PER_CHUNK; i++)
	{
		Block currentBlock = m_blocks[i];
		if (currentBlock.GetDefinition()->m_isOpaque || currentBlock.m_type == BlockType::AIR)
		{
			continue;
		}

		WorldPosition coords = GetWorldPositionForBlockIndex(i);
		Vertex_PCT vertex;
		vertex.color = RGBA(0x000000FF);
		BlockDefinition* currentDefinition = currentBlock.GetDefinition();
		AABB2 topTex = currentDefinition->GetTopIndex();
		AABB2 sideTex = currentDefinition->GetSideIndex();
		AABB2 bottomTex = currentDefinition->GetBottomIndex();

		BlockInfo info = GetBlockInfoFromLocalCoords(GetLocalCoordsFromBlockIndex(i));

		BlockInfo belowInfo = info.GetBelow();
		if (belowInfo.IsValid())
		{
			Block* belowBlock = belowInfo.GetBlock();
			BlockDefinition* belowType = belowBlock->GetDefinition();
			if (belowBlock && !belowType->m_isOpaque && belowBlock->m_type != currentBlock.m_type)
			{
				vertex.color = RGBA(belowBlock->GetDampedLightValue(0x33));
				vertex.texCoords = bottomTex.mins;
				vertex.pos = Vector3(coords.x, coords.y, coords.z);
				m_vertexArray.push_back(vertex);
				vertex.texCoords = Vector2(bottomTex.maxs.x, bottomTex.mins.y);
				vertex.pos = Vector3(coords.x, coords.y + blockSize, coords.z);
				m_vertexArray.push_back(vertex);
				vertex.texCoords = bottomTex.maxs;
				vertex.pos = Vector3(coords.x + blockSize, coords.y + blockSize, coords.z);
				m_vertexArray.push_back(vertex);
				vertex.texCoords = Vector2(bottomTex.mins.x, bottomTex.maxs.y);
				vertex.pos = Vector3(coords.x + blockSize, coords.y, coords.z);
				m_vertexArray.push_back(vertex);
			}
		}

		BlockInfo aboveInfo = info.GetAbove();
		if (aboveInfo.IsValid())
		{
			Block* aboveBlock = aboveInfo.GetBlock();
			BlockDefinition* aboveType = aboveBlock->GetDefinition();
			if (aboveBlock && !aboveType->m_isOpaque && aboveBlock->m_type != currentBlock.m_type)
			{
				vertex.color = RGBA(aboveBlock->GetLightValue());
				vertex.texCoords = Vector2(topTex.mins.x, topTex.mins.y);
				vertex.pos = Vector3(coords.x, coords.y, coords.z + blockSize);
				m_vertexArray.push_back(vertex);
				vertex.texCoords = Vector2(topTex.maxs.x, topTex.mins.y);
				vertex.pos = Vector3(coords.x + blockSize, coords.y, coords.z + blockSize);
				m_vertexArray.push_back(vertex);
				vertex.texCoords = Vector2(topTex.maxs.x, topTex.maxs.y);
				vertex.pos = Vector3(coords.x + blockSize, coords.y + blockSize, coords.z + blockSize);
				m_vertexArray.push_back(vertex);
				vertex.texCoords = Vector2(topTex.mins.x, topTex.maxs.y);
				vertex.pos = Vector3(coords.x, coords.y + blockSize, coords.z + blockSize);
				m_vertexArray.push_back(vertex);
			}
		}

		BlockInfo westInfo = info.GetWest();
		if (westInfo.IsValid())
		{
			Block* westBlock = westInfo.GetBlock();
			BlockDefinition* westType = westBlock->GetDefinition();
			if (westBlock && !westType->m_isOpaque && westBlock->m_type != currentBlock.m_type)
			{
				vertex.color = RGBA(westBlock->GetDampedLightValue(0x22));
				vertex.texCoords = Vector2(sideTex.mins.x, sideTex.mins.y);
				vertex.pos = Vector3(coords.x, coords.y + blockSize, coords.z);
				m_vertexArray.push_back(vertex);
				vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.mins.y);
				vertex.pos = Vector3(coords.x, coords.y, coords.z);
				m_vertexArray.push_back(vertex);
				vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.maxs.y);
				vertex.pos = Vector3(coords.x, coords.y, coords.z + blockSize);
				m_vertexArray.push_back(vertex);
				vertex.texCoords = Vector2(sideTex.mins.x, sideTex.maxs.y);
				vertex.pos = Vector3(coords.x, coords.y + blockSize, coords.z + blockSize);
				m_vertexArray.push_back(vertex);
			}
		}

		BlockInfo eastInfo = info.GetEast();
		if (eastInfo.IsValid())
		{
			Block* eastBlock = eastInfo.GetBlock();
			BlockDefinition* eastType = eastBlock->GetDefinition();
			if (eastBlock && !eastType->m_isOpaque && eastBlock->m_type != currentBlock.m_type)
			{
				vertex.color = RGBA(eastBlock->GetDampedLightValue(0x22));
				vertex.texCoords = Vector2(sideTex.mins.x, sideTex.mins.y);
				vertex.pos = Vector3(coords.x + blockSize, coords.y, coords.z);
				m_vertexArray.push_back(vertex);
				vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.mins.y);
				vertex.pos = Vector3(coords.x + blockSize, coords.y + blockSize, coords.z);
				m_vertexArray.push_back(vertex);
				vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.maxs.y);
				vertex.pos = Vector3(coords.x + blockSize, coords.y + blockSize, coords.z + blockSize);
				m_vertexArray.push_back(vertex);
				vertex.texCoords = Vector2(sideTex.mins.x, sideTex.maxs.y);
				vertex.pos = Vector3(coords.x + blockSize, coords.y, coords.z + blockSize);
				m_vertexArray.push_back(vertex);
			}
		}

		BlockInfo southInfo = info.GetSouth();
		if (southInfo.IsValid())
		{
			Block* southBlock = southInfo.GetBlock();
			BlockDefinition* southType = southBlock->GetDefinition();
			if (southBlock && !southType->m_isOpaque && southBlock->m_type != currentBlock.m_type)
			{
				vertex.color = RGBA(southBlock->GetDampedLightValue(0x11));
				vertex.texCoords = Vector2(sideTex.mins.x, sideTex.mins.y);
				vertex.pos = Vector3(coords.x, coords.y, coords.z);
				m_vertexArray.push_back(vertex);
				vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.mins.y);
				vertex.pos = Vector3(coords.x + blockSize, coords.y, coords.z);
				m_vertexArray.push_back(vertex);
				vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.maxs.y);
				vertex.pos = Vector3(coords.x + blockSize, coords.y, coords.z + blockSize);
				m_vertexArray.push_back(vertex);
				vertex.texCoords = Vector2(sideTex.mins.x, sideTex.maxs.y);
				vertex.pos = Vector3(coords.x, coords.y, coords.z + blockSize);
				m_vertexArray.push_back(vertex);
			}
		}

		BlockInfo northInfo = info.GetNorth();
		if (northInfo.IsValid())
		{
			Block* northBlock = northInfo.GetBlock();
			BlockDefinition* northType = northBlock->GetDefinition();
			if (northBlock && !northType->m_isOpaque && northBlock->m_type != currentBlock.m_type)
			{
				vertex.color = RGBA(northBlock->GetDampedLightValue(0x11));
				vertex.texCoords = Vector2(sideTex.mins.x, sideTex.mins.y);
				vertex.pos = Vector3(coords.x + blockSize, coords.y + blockSize, coords.z);
				m_vertexArray.push_back(vertex);
				vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.mins.y);
				vertex.pos = Vector3(coords.x, coords.y + blockSize, coords.z);
				m_vertexArray.push_back(vertex);
				vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.maxs.y);
				vertex.pos = Vector3(coords.x, coords.y + blockSize, coords.z + blockSize);
				m_vertexArray.push_back(vertex);
				vertex.texCoords = Vector2(sideTex.mins.x, sideTex.maxs.y);
				vertex.pos = Vector3(coords.x + blockSize, coords.y + blockSize, coords.z + blockSize);
				m_vertexArray.push_back(vertex);
			}
		}
	}
	m_numVerts = m_vertexArray.size();
	TheRenderer::instance->BindAndBufferVBOData(m_vboID, m_vertexArray.data(), m_numVerts);
	EndTiming(g_vaBuildingProfiling);
}

void Chunk::GenerateSaveData(std::vector<unsigned char>& data)
{
	uchar currentType = m_blocks[0].m_type;
	uchar numOfType = 0;
	data.push_back(currentType);
	for (Block block : m_blocks)
	{
		if (block.m_type == currentType && numOfType < 255)
		{
			++numOfType;
		}
		else
		{
			data.push_back(numOfType);
			currentType = block.m_type;
			data.push_back(currentType);
			numOfType = 1;
		}
	}
	data.push_back(numOfType);
}

void Chunk::LoadChunkFromData(std::vector<unsigned char>& data)
{
	int currentIndex = 0;
	for (unsigned int i = 0; i < data.size(); i += 2)
	{
		uchar blockType = data[i];
		int numBlocks = data[i + 1];
		for (int j = 0; j < numBlocks; j++)
		{
			m_blocks[currentIndex++].m_type = blockType;
		}
	}
}

void Chunk::SetDirtyFlagAndAddToDirtyList(LocalIndex blockToDirtyIndex)
{
	Block* blockToDirty = GetBlock(blockToDirtyIndex);
	blockToDirty->SetDirty(true);
	m_world->m_dirtyBlocks.emplace_back(BlockInfo(this, blockToDirtyIndex));
}

void Chunk::SetEdgeBits()
{
	for (int index = 0; index < BLOCKS_PER_CHUNK; index++)
	{
		if (((index & LOCAL_X_MASK) == LOCAL_X_MASK)
			|| ((index & LOCAL_X_MASK) == 0x00)
			|| ((index & LOCAL_Y_MASK) == LOCAL_Y_MASK)
			|| ((index & LOCAL_Y_MASK) == 0x00))
		{
			GetBlock(index)->SetEdgeBlock(true);
		}
	}
}

void Chunk::FlagEdgesAsDirtyLighting()
{
	for (int index = 0; index < BLOCKS_PER_CHUNK; index++)
	{
		Block* block = GetBlock(index);
		if (block->IsEdgeBlock() && !BlockDefinition::GetDefinition(block->m_type)->m_isOpaque)
		{
			SetDirtyFlagAndAddToDirtyList(index);
		}
	}
}
