#include "Chunk.hpp"
#include "Engine/Math/Vector3Int.hpp"
#include "Engine/Renderer/Vertex.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/BlockDefinition.h"
#include "Game/World.hpp"
#include "Game/Generator.hpp"
#include <map>

//-----------------------------------------------------------------------------------
Chunk::Chunk(const ChunkCoords& chunkCoords, World* world)
: m_chunkPosition(chunkCoords)
, m_bottomLeftCorner(WorldPosition(static_cast<float>((m_chunkPosition.x * BLOCKS_WIDE_X)), static_cast<float>((m_chunkPosition.y * BLOCKS_WIDE_Y)), 0.0f))
, m_eastChunk(nullptr)
, m_westChunk(nullptr)
, m_southChunk(nullptr)
, m_northChunk(nullptr)
, m_isDirty(false)
, m_world(world)
, m_vboID(0)
, m_numVerts(0)
{
	//REMINDER: THREAD-SAFE CODE ONLY!
	memset(m_blocks, 0, sizeof(m_blocks[0]) * BLOCKS_PER_CHUNK);
	GenerateChunk();
	SetEdgeBits();
}

//-----------------------------------------------------------------------------------
Chunk::Chunk(const ChunkCoords& chunkCoords, std::vector<unsigned char>& data, World* world)
: m_chunkPosition(chunkCoords)
, m_bottomLeftCorner(WorldPosition(static_cast<float>((m_chunkPosition.x * BLOCKS_WIDE_X)), static_cast<float>((m_chunkPosition.y * BLOCKS_WIDE_Y)), 0.0f))
, m_eastChunk(nullptr)
, m_westChunk(nullptr)
, m_southChunk(nullptr)
, m_northChunk(nullptr)
, m_isDirty(false)
, m_world(world)
, m_vboID(0)
, m_numVerts(0)
{
	//REMINDER: THREAD-SAFE CODE ONLY!
	memset(m_blocks, 0, sizeof(m_blocks[0]) * BLOCKS_PER_CHUNK);
	LoadChunkFromData(data);
	SetEdgeBits();
}

//-----------------------------------------------------------------------------------
Chunk::~Chunk()
{
	DebuggerPrintf("[%i] World [%i]: Deleting Chunk %i,%i\n", g_frameNumber, m_world->m_worldID, m_chunkPosition.x, m_chunkPosition.y);
	TheRenderer::instance->DeleteBuffers(m_vboID);
}

//-----------------------------------------------------------------------------------
void Chunk::Update(float deltaTime)
{
	UNUSED(deltaTime);
}

//-----------------------------------------------------------------------------------
void Chunk::Render() const
{
	TheRenderer::instance->DrawVBO_PCT(m_vboID, m_numVerts, TheRenderer::QUADS, TheGame::instance->m_blockSheet->GetTexture());
}

//-----------------------------------------------------------------------------------
void Chunk::GenerateChunk()
{
	StartTiming(g_generationProfiling);
	m_world->m_generator->GenerateChunk(m_blocks, this);
	EndTiming(g_generationProfiling);
}

//-----------------------------------------------------------------------------------
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
					currentBlock->SetLightValue(m_world->m_skyLight);
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
			SetBlockDirtyAndAddToDirtyList(i);
		}
	}
}

//-----------------------------------------------------------------------------------
WorldCoords Chunk::GetWorldCoordsForBlockIndex(LocalIndex index) const
{
	LocalCoords coords = GetLocalCoordsFromBlockIndex(index);
	return WorldCoords(coords.x + static_cast<int>(m_bottomLeftCorner.x), coords.y + static_cast<int>(m_bottomLeftCorner.y), coords.z);
}

//-----------------------------------------------------------------------------------
LocalIndex Chunk::GetBlockIndexFromWorldCoords(const WorldCoords& coords) const
{
	LocalCoords localCoords = GetLocalCoordsFromWorldCoords(coords);
	return GetBlockIndexFromLocalCoords(localCoords);
}

//-----------------------------------------------------------------------------------
LocalCoords Chunk::GetLocalCoordsFromWorldCoords(const WorldCoords &coords) const
{
	LocalCoords localCoords = LocalCoords(coords.x - static_cast<int>(m_bottomLeftCorner.x), coords.y - static_cast<int>(m_bottomLeftCorner.y), coords.z);
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
//-----------------------------------------------------------------------------------
LocalCoords Chunk::GetLocalCoordsFromWorldPosition(const WorldPosition &pos) const
{
	return GetLocalCoordsFromWorldCoords(WorldCoords(static_cast<int>(floor(pos.x)), static_cast<int>(floor(pos.y)), static_cast<int>(floor(pos.z))));
}

//-----------------------------------------------------------------------------------
LocalIndex Chunk::GetBlockIndexFromWorldPosition(const WorldPosition& pos) const
{
	return GetBlockIndexFromWorldCoords(WorldCoords(static_cast<int>(floor(pos.x)), static_cast<int>(floor(pos.y)), static_cast<int>(floor(pos.z))));
}

//-----------------------------------------------------------------------------------
bool Chunk::IsInFrustum(const Vector3& cameraXYZ, const WorldPosition& playerPosition) const
{
	const Vector3 chunkMins = Vector3(static_cast<float>(m_chunkPosition.x * Chunk::BLOCKS_WIDE_X), static_cast<float>(m_chunkPosition.y * Chunk::BLOCKS_WIDE_Y), 0.0f);
	const Vector3 chunkOffsetX = Vector3::UNIT_X * static_cast<float>(Chunk::BLOCKS_WIDE_X);
	const Vector3 chunkOffsetY = Vector3::UNIT_Y * static_cast<float>(Chunk::BLOCKS_WIDE_Y);
	const Vector3 chunkOffsetZ = Vector3::UNIT_Z * static_cast<float>(Chunk::BLOCKS_TALL_Z);
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

//-----------------------------------------------------------------------------------
void Chunk::GenerateVertexArray()
{
	DebuggerPrintf("[%i] World [%i]: Building Chunk %i,%i VA\n", g_frameNumber, m_world->m_worldID, m_chunkPosition.x, m_chunkPosition.y);
	StartTiming(g_vaBuildingProfiling);
	if (m_vboID == 0)
	{
		m_vboID = TheRenderer::instance->GenerateBufferID();
	}
	std::vector<Vertex_PCT> tempVertexArray;
	tempVertexArray.reserve(5000);
	const float blockSize = 1.0f;
	for (int i = 0; i < BLOCKS_PER_CHUNK; i++)
	{
		Block currentBlock = m_blocks[i];
		if (!currentBlock.GetDefinition()->m_isOpaque)
		{
			continue;
		}

		WorldPosition coords = GetWorldMinsForBlockIndex(i);
		Vertex_PCT vertex;
		vertex.color = RGBA(0x000000FF);
		BlockDefinition* currentDefinition = BlockDefinition::GetDefinition(currentBlock.m_type);
		AABB2 topTex = currentDefinition->GetTopIndex();
		AABB2 sideTex = currentDefinition->GetSideIndex();
		AABB2 bottomTex = currentDefinition->GetBottomIndex();

		Block* belowBlock = GetBelow(i);
		if (belowBlock && !BlockDefinition::GetDefinition(belowBlock->m_type)->m_isOpaque)
		{
			vertex.color = currentBlock.HasBelowPortal() ? RGBA::VAPORWAVE : RGBA(belowBlock->GetDampedLightValue(0x33));
			vertex.texCoords = bottomTex.mins;
			vertex.pos = Vector3(coords.x, coords.y, coords.z);
			tempVertexArray.push_back(vertex);
			vertex.texCoords = Vector2(bottomTex.maxs.x, bottomTex.mins.y);
			vertex.pos = Vector3(coords.x, coords.y + blockSize, coords.z);
			tempVertexArray.push_back(vertex);
			vertex.texCoords = bottomTex.maxs;
			vertex.pos = Vector3(coords.x + blockSize, coords.y + blockSize, coords.z);
			tempVertexArray.push_back(vertex);
			vertex.texCoords = Vector2(bottomTex.mins.x, bottomTex.maxs.y);
			vertex.pos = Vector3(coords.x + blockSize, coords.y, coords.z);
			tempVertexArray.push_back(vertex);
		}

		Block* aboveBlock = GetAbove(i);
		if (aboveBlock && !BlockDefinition::GetDefinition(aboveBlock->m_type)->m_isOpaque)
		{
			vertex.color = currentBlock.HasAbovePortal() ? RGBA::VAPORWAVE : RGBA(aboveBlock->GetLightValue());
			vertex.texCoords = Vector2(topTex.mins.x, topTex.mins.y);
			vertex.pos = Vector3(coords.x, coords.y, coords.z + blockSize);
			tempVertexArray.push_back(vertex);
			vertex.texCoords = Vector2(topTex.maxs.x, topTex.mins.y);
			vertex.pos = Vector3(coords.x + blockSize, coords.y, coords.z + blockSize);
			tempVertexArray.push_back(vertex);
			vertex.texCoords = Vector2(topTex.maxs.x, topTex.maxs.y);
			vertex.pos = Vector3(coords.x + blockSize, coords.y + blockSize, coords.z + blockSize);
			tempVertexArray.push_back(vertex);
			vertex.texCoords = Vector2(topTex.mins.x, topTex.maxs.y);
			vertex.pos = Vector3(coords.x, coords.y + blockSize, coords.z + blockSize);
			tempVertexArray.push_back(vertex);
		}

		Block* westBlock = GetWest(i);
		if (westBlock && !BlockDefinition::GetDefinition(westBlock->m_type)->m_isOpaque)
		{
			vertex.color = currentBlock.HasWestPortal() ? RGBA::VAPORWAVE : RGBA(westBlock->GetDampedLightValue(0x22));
			vertex.texCoords = Vector2(sideTex.mins.x, sideTex.mins.y);
			vertex.pos = Vector3(coords.x, coords.y + blockSize, coords.z);
			tempVertexArray.push_back(vertex);
			vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.mins.y);
			vertex.pos = Vector3(coords.x, coords.y, coords.z);
			tempVertexArray.push_back(vertex);
			vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.maxs.y);
			vertex.pos = Vector3(coords.x, coords.y, coords.z + blockSize);
			tempVertexArray.push_back(vertex);
			vertex.texCoords = Vector2(sideTex.mins.x, sideTex.maxs.y);
			vertex.pos = Vector3(coords.x, coords.y + blockSize, coords.z + blockSize);
			tempVertexArray.push_back(vertex);
		}

		Block* eastBlock = GetEast(i);
		if (eastBlock && !BlockDefinition::GetDefinition(eastBlock->m_type)->m_isOpaque)
		{
			vertex.color = currentBlock.HasEastPortal() ? RGBA::VAPORWAVE : RGBA(eastBlock->GetDampedLightValue(0x22));
			vertex.texCoords = Vector2(sideTex.mins.x, sideTex.mins.y);
			vertex.pos = Vector3(coords.x + blockSize, coords.y, coords.z);
			tempVertexArray.push_back(vertex);
			vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.mins.y);
			vertex.pos = Vector3(coords.x + blockSize, coords.y + blockSize, coords.z);
			tempVertexArray.push_back(vertex);
			vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.maxs.y);
			vertex.pos = Vector3(coords.x + blockSize, coords.y + blockSize, coords.z + blockSize);
			tempVertexArray.push_back(vertex);
			vertex.texCoords = Vector2(sideTex.mins.x, sideTex.maxs.y);
			vertex.pos = Vector3(coords.x + blockSize, coords.y, coords.z + blockSize);
			tempVertexArray.push_back(vertex);
		}

		Block* southBlock = GetSouth(i);
		if (southBlock && !BlockDefinition::GetDefinition(southBlock->m_type)->m_isOpaque)
		{
			vertex.color = currentBlock.HasSouthPortal() ? RGBA::VAPORWAVE : RGBA(southBlock->GetDampedLightValue(0x11));
			vertex.texCoords = Vector2(sideTex.mins.x, sideTex.mins.y);
			vertex.pos = Vector3(coords.x, coords.y, coords.z);
			tempVertexArray.push_back(vertex);
			vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.mins.y);
			vertex.pos = Vector3(coords.x + blockSize, coords.y, coords.z);
			tempVertexArray.push_back(vertex);
			vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.maxs.y);
			vertex.pos = Vector3(coords.x + blockSize, coords.y, coords.z + blockSize);
			tempVertexArray.push_back(vertex);
			vertex.texCoords = Vector2(sideTex.mins.x, sideTex.maxs.y);
			vertex.pos = Vector3(coords.x, coords.y, coords.z + blockSize);
			tempVertexArray.push_back(vertex);
		}

		Block* northBlock = GetNorth(i);
		if (northBlock && !BlockDefinition::GetDefinition(northBlock->m_type)->m_isOpaque)
		{
			vertex.color = currentBlock.HasNorthPortal() ? RGBA::VAPORWAVE : RGBA(northBlock->GetDampedLightValue(0x11));
			vertex.texCoords = Vector2(sideTex.mins.x, sideTex.mins.y);
			vertex.pos = Vector3(coords.x + blockSize, coords.y + blockSize, coords.z);
			tempVertexArray.push_back(vertex);
			vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.mins.y);
			vertex.pos = Vector3(coords.x, coords.y + blockSize, coords.z);
			tempVertexArray.push_back(vertex);
			vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.maxs.y);
			vertex.pos = Vector3(coords.x, coords.y + blockSize, coords.z + blockSize);
			tempVertexArray.push_back(vertex);
			vertex.texCoords = Vector2(sideTex.mins.x, sideTex.maxs.y);
			vertex.pos = Vector3(coords.x + blockSize, coords.y + blockSize, coords.z + blockSize);
			tempVertexArray.push_back(vertex);
		}
	}

	//Opaque drawing
	for (int i = 0; i < BLOCKS_PER_CHUNK; i++)
	{
		Block currentBlock = m_blocks[i];
		if (currentBlock.GetDefinition()->m_isOpaque || currentBlock.m_type == BlockType::AIR)
		{
			continue;
		}

		WorldPosition coords = GetWorldMinsForBlockIndex(i);
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
				tempVertexArray.push_back(vertex);
				vertex.texCoords = Vector2(bottomTex.maxs.x, bottomTex.mins.y);
				vertex.pos = Vector3(coords.x, coords.y + blockSize, coords.z);
				tempVertexArray.push_back(vertex);
				vertex.texCoords = bottomTex.maxs;
				vertex.pos = Vector3(coords.x + blockSize, coords.y + blockSize, coords.z);
				tempVertexArray.push_back(vertex);
				vertex.texCoords = Vector2(bottomTex.mins.x, bottomTex.maxs.y);
				vertex.pos = Vector3(coords.x + blockSize, coords.y, coords.z);
				tempVertexArray.push_back(vertex);
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
				tempVertexArray.push_back(vertex);
				vertex.texCoords = Vector2(topTex.maxs.x, topTex.mins.y);
				vertex.pos = Vector3(coords.x + blockSize, coords.y, coords.z + blockSize);
				tempVertexArray.push_back(vertex);
				vertex.texCoords = Vector2(topTex.maxs.x, topTex.maxs.y);
				vertex.pos = Vector3(coords.x + blockSize, coords.y + blockSize, coords.z + blockSize);
				tempVertexArray.push_back(vertex);
				vertex.texCoords = Vector2(topTex.mins.x, topTex.maxs.y);
				vertex.pos = Vector3(coords.x, coords.y + blockSize, coords.z + blockSize);
				tempVertexArray.push_back(vertex);
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
				tempVertexArray.push_back(vertex);
				vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.mins.y);
				vertex.pos = Vector3(coords.x, coords.y, coords.z);
				tempVertexArray.push_back(vertex);
				vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.maxs.y);
				vertex.pos = Vector3(coords.x, coords.y, coords.z + blockSize);
				tempVertexArray.push_back(vertex);
				vertex.texCoords = Vector2(sideTex.mins.x, sideTex.maxs.y);
				vertex.pos = Vector3(coords.x, coords.y + blockSize, coords.z + blockSize);
				tempVertexArray.push_back(vertex);
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
				tempVertexArray.push_back(vertex);
				vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.mins.y);
				vertex.pos = Vector3(coords.x + blockSize, coords.y + blockSize, coords.z);
				tempVertexArray.push_back(vertex);
				vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.maxs.y);
				vertex.pos = Vector3(coords.x + blockSize, coords.y + blockSize, coords.z + blockSize);
				tempVertexArray.push_back(vertex);
				vertex.texCoords = Vector2(sideTex.mins.x, sideTex.maxs.y);
				vertex.pos = Vector3(coords.x + blockSize, coords.y, coords.z + blockSize);
				tempVertexArray.push_back(vertex);
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
				tempVertexArray.push_back(vertex);
				vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.mins.y);
				vertex.pos = Vector3(coords.x + blockSize, coords.y, coords.z);
				tempVertexArray.push_back(vertex);
				vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.maxs.y);
				vertex.pos = Vector3(coords.x + blockSize, coords.y, coords.z + blockSize);
				tempVertexArray.push_back(vertex);
				vertex.texCoords = Vector2(sideTex.mins.x, sideTex.maxs.y);
				vertex.pos = Vector3(coords.x, coords.y, coords.z + blockSize);
				tempVertexArray.push_back(vertex);
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
				tempVertexArray.push_back(vertex);
				vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.mins.y);
				vertex.pos = Vector3(coords.x, coords.y + blockSize, coords.z);
				tempVertexArray.push_back(vertex);
				vertex.texCoords = Vector2(sideTex.maxs.x, sideTex.maxs.y);
				vertex.pos = Vector3(coords.x, coords.y + blockSize, coords.z + blockSize);
				tempVertexArray.push_back(vertex);
				vertex.texCoords = Vector2(sideTex.mins.x, sideTex.maxs.y);
				vertex.pos = Vector3(coords.x + blockSize, coords.y + blockSize, coords.z + blockSize);
				tempVertexArray.push_back(vertex);
			}
		}
	}

	m_numVerts = tempVertexArray.size();
	TheRenderer::instance->BindAndBufferVBOData(m_vboID, tempVertexArray.data(), m_numVerts);
	m_isDirty = false;
	EndTiming(g_vaBuildingProfiling);
}

//-----------------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------------
void Chunk::SetBlockDirtyAndAddToDirtyList(LocalIndex blockToDirtyIndex)
{
	Block* blockToDirty = GetBlock(blockToDirtyIndex);
	blockToDirty->SetDirty(true);
	m_world->m_dirtyBlocks.emplace_back(BlockInfo(this, blockToDirtyIndex));
}

//-----------------------------------------------------------------------------------
void Chunk::DirtyAndAddToDirtyList()
{
	//Don't do anything if we're already in the list.
	if (m_isDirty == true)
	{
		return;
	}
	m_isDirty = true;
	m_world->m_dirtyChunks.emplace(this, m_world->DistanceSquaredFromPlayerToChunk(this->m_chunkPosition));
}

//-----------------------------------------------------------------------------------
void Chunk::SetHighPriorityChunkDirtyAndAddToDirtyList()
{
	m_isDirty = true;
	m_world->m_dirtyChunks.emplace(this, 0.0f);
}

//-----------------------------------------------------------------------------------
void Chunk::SetEdgeBits()
{
	//Directions are relative to the top layer of the chunk, viewed from above.
	const int TOP_LEFT_INDEX = BLOCKS_PER_CHUNK - BLOCKS_WIDE_X;
	const int TOP_RIGHT_INDEX = BLOCKS_PER_CHUNK - 1;
	const int BOTTOM_RIGHT_INDEX_MINUS_1 = BLOCKS_PER_CHUNK - BLOCKS_PER_LAYER + (BLOCKS_WIDE_X - 2);
	const int TOP_RIGHT_INDEX_MINUS_1 = BLOCKS_PER_CHUNK - 2;
	//West Side
	for (int index = 0; index < TOP_LEFT_INDEX; index += BLOCKS_WIDE_Y)
	{
		GetBlock(index)->SetEdgeBlock(true);
	}
	//East Side
	for (int index = BLOCKS_WIDE_X - 1; index < TOP_RIGHT_INDEX; index += BLOCKS_WIDE_Y)
	{
		GetBlock(index)->SetEdgeBlock(true);
	}
	//North Side
	for (int index = BLOCKS_PER_LAYER - BLOCKS_WIDE_X + 1; index < TOP_RIGHT_INDEX_MINUS_1; index += BLOCKS_PER_LAYER)
	{
		const int BLOCKS_PER_ITERATION = BLOCKS_WIDE_X - 2;
		for (int j = 0; j < BLOCKS_PER_ITERATION; j++)
		{
			GetBlock(index + j)->SetEdgeBlock(true);
		}
	}
	//South Side
	for (int index = 1; index < BOTTOM_RIGHT_INDEX_MINUS_1; index += BLOCKS_PER_LAYER)
	{
		const int BLOCKS_PER_ITERATION = BLOCKS_WIDE_X - 2;
		for (int j = 0; j < BLOCKS_PER_ITERATION; j++)
		{
			GetBlock(index + j)->SetEdgeBlock(true);
		}
	}
}

//-----------------------------------------------------------------------------------
void Chunk::FlagEdgesAsDirtyLighting(Direction dir)
{
	//Directions are relative to the top layer of the chunk, viewed from above.
	const int TOP_LEFT_INDEX = BLOCKS_PER_CHUNK - BLOCKS_WIDE_X;
	const int TOP_RIGHT_INDEX = BLOCKS_PER_CHUNK - 1;
	const int BOTTOM_RIGHT_INDEX_MINUS_1 = BLOCKS_PER_CHUNK - BLOCKS_PER_LAYER + (BLOCKS_WIDE_X - 2);
	const int TOP_RIGHT_INDEX_MINUS_1 = BLOCKS_PER_CHUNK - 2;
	switch (dir)
	{
	case NORTH:
		//North Side
		for (int index = BLOCKS_PER_LAYER - BLOCKS_WIDE_X + 1; index < TOP_RIGHT_INDEX_MINUS_1; index += BLOCKS_PER_LAYER)
		{
			const int BLOCKS_PER_ITERATION = BLOCKS_WIDE_X - 2;
			for (int j = 0; j < BLOCKS_PER_ITERATION; j++)
			{
				Block* block = GetBlock(index + j);
				if (block->IsSky() || BlockDefinition::GetDefinition(block->m_type)->IsIlluminated())
				{
					SetBlockDirtyAndAddToDirtyList(index + j);
				}
			}
		}
		break;
	case SOUTH:
		//South Side
		for (int index = 1; index < BOTTOM_RIGHT_INDEX_MINUS_1; index += BLOCKS_PER_LAYER)
		{
			const int BLOCKS_PER_ITERATION = BLOCKS_WIDE_X - 2;
			for (int j = 0; j < BLOCKS_PER_ITERATION; j++)
			{
				Block* block = GetBlock(index + j);
				if (block->IsSky() || BlockDefinition::GetDefinition(block->m_type)->IsIlluminated())
				{
					SetBlockDirtyAndAddToDirtyList(index + j);
				}
			}
		}
		break;
	case EAST:
		//East Side
		for (int index = BLOCKS_WIDE_X - 1; index < TOP_RIGHT_INDEX; index += BLOCKS_WIDE_Y)
		{
			Block* block = GetBlock(index);
			if (block->IsSky() || BlockDefinition::GetDefinition(block->m_type)->IsIlluminated())
			{
				SetBlockDirtyAndAddToDirtyList(index);
			}
		}
		break;
	case WEST:
		//West Side
		for (int index = 0; index < TOP_LEFT_INDEX; index += BLOCKS_WIDE_Y)
		{
			Block* block = GetBlock(index);
			if (block->IsSky() || BlockDefinition::GetDefinition(block->m_type)->IsIlluminated())
			{
				SetBlockDirtyAndAddToDirtyList(index);
			}
		}
		break;
	default:
		break;
	}
}