#pragma once

#include "Game/Block.hpp"
#include "Game/BlockInfo.hpp"
#include "GameCommon.hpp"
#include <vector>
class Vector2Int;
struct Vertex_PCT;

class Chunk
{
public:
	Chunk(const ChunkCoords& chunkCoords, World* world);
	Chunk(const ChunkCoords& chunkCoords, std::vector<unsigned char>& data, World* world);
	~Chunk();
	void Update(float deltaTime);
	void Render() const;
	void UpdateVAIfDirty();
	void GenerateChunk();
	void CalculateSkyLighting();
	inline Block* GetBlock(LocalIndex index);
	inline Block* GetAbove(LocalIndex index);
	inline Block* GetBelow(LocalIndex index);
	inline Block* GetNorth(LocalIndex index);
	inline Block* GetSouth(LocalIndex index);
	inline Block* GetEast(LocalIndex index);
	inline Block* GetWest(LocalIndex index);
	inline LocalCoords GetLocalCoordsFromBlockIndex(LocalIndex index) const;
	LocalCoords GetLocalCoordsFromWorldCoords(const WorldCoords &coords) const;
	LocalCoords GetLocalCoordsFromWorldPosition(const WorldPosition &pos) const;
	WorldCoords GetWorldCoordsForBlockIndex(LocalIndex index) const;
	inline WorldPosition GetWorldPositionForBlockIndex(LocalIndex index) const;
	inline LocalIndex GetBlockIndexFromLocalCoords(const LocalCoords& coords) const;
	inline BlockInfo GetBlockInfoFromLocalCoords(const LocalCoords& coords);
	LocalIndex GetBlockIndexFromWorldCoords(const WorldCoords& pos) const;
	LocalIndex GetBlockIndexFromWorldPosition(const WorldPosition& pos) const;
	bool IsInFrustum(const Vector3& cameraXYZ, const WorldPosition& playerPosition) const;
	void GenerateVertexArray();
	void GenerateSaveData(std::vector<unsigned char>& data);
	void LoadChunkFromData(std::vector<unsigned char>& data);
	void SetDirtyFlagAndAddToDirtyList(LocalIndex blockToDirtyIndex);
	void SetEdgeBits();
	void FlagEdgesAsDirtyLighting();
	static const int CHUNK_BITS_X = 4;
	static const int CHUNK_BITS_Y = 4;
	static const int CHUNK_BITS_XY = CHUNK_BITS_X + CHUNK_BITS_Y;
	static const int CHUNK_BITS_Z = 7;
	static const int BLOCKS_WIDE_X = BIT(CHUNK_BITS_X);
	static const int BLOCKS_WIDE_Y = BIT(CHUNK_BITS_Y);
	static const int BLOCKS_TALL_Z = BIT(CHUNK_BITS_Z);
	static const int BLOCKS_PER_LAYER = BLOCKS_WIDE_X * BLOCKS_WIDE_Y;
	static const int BLOCKS_PER_CHUNK = BLOCKS_PER_LAYER * BLOCKS_TALL_Z;
	ChunkCoords m_chunkPosition;
	WorldPosition m_bottomLeftCorner;
	Chunk* m_eastChunk;
	Chunk* m_westChunk;
	Chunk* m_northChunk;
	Chunk* m_southChunk;
	World* m_world;
	bool m_isDirty;
	static const int LOCAL_X_MASK = BLOCKS_WIDE_X - 1;
	static const int LOCAL_Y_MASK = (BLOCKS_WIDE_Y - 1) << CHUNK_BITS_X;
	static const int LOCAL_Z_MASK = (BLOCKS_TALL_Z - 1) << CHUNK_BITS_XY;
private:
	Block m_blocks[BLOCKS_PER_CHUNK];
	int m_vboID;
	int m_numVerts;

};

inline WorldPosition Chunk::GetWorldPositionForBlockIndex(LocalIndex index) const
{
	LocalCoords coords = GetLocalCoordsFromBlockIndex(index);
	return WorldPosition(coords.x + m_bottomLeftCorner.x, coords.y + m_bottomLeftCorner.y, coords.z);
}

inline LocalCoords Chunk::GetLocalCoordsFromBlockIndex(LocalIndex index) const
{
	return LocalCoords(index & LOCAL_X_MASK, (index & LOCAL_Y_MASK) >> CHUNK_BITS_X, index >> CHUNK_BITS_XY);
}

inline LocalIndex Chunk::GetBlockIndexFromLocalCoords(const LocalCoords& coords) const
{
	return (coords.z << CHUNK_BITS_XY) + (coords.y << CHUNK_BITS_X) + coords.x;
}

inline BlockInfo Chunk::GetBlockInfoFromLocalCoords(const LocalCoords& coords) 
{
	return BlockInfo(this, GetBlockIndexFromLocalCoords(coords));
}

inline Block* Chunk::GetBlock(LocalIndex index)
{
	if (index > BLOCKS_PER_CHUNK || index < 0)
	{
		return nullptr;
	}
	return &m_blocks[index];
}

inline Block* Chunk::GetAbove(LocalIndex index)
{
	if ((index & LOCAL_Z_MASK) == LOCAL_Z_MASK)
	{
		return nullptr;
	}
	return &(m_blocks[index + BLOCKS_PER_LAYER]);
}

inline Block* Chunk::GetBelow(LocalIndex index)
{
	if ((index & LOCAL_Z_MASK) == 0x00)
	{
		return nullptr;
	}
	return &(m_blocks[index - BLOCKS_PER_LAYER]);
}

inline Block* Chunk::GetNorth(LocalIndex index)
{
	if ((index & LOCAL_Y_MASK) == LOCAL_Y_MASK)
	{
		if (m_northChunk)
		{
			LocalCoords blockOffsetCoords = GetLocalCoordsFromBlockIndex(index);
			blockOffsetCoords.y = 0;
			LocalIndex neighborBlockIndex = GetBlockIndexFromLocalCoords(blockOffsetCoords);
			return m_northChunk->GetBlock(neighborBlockIndex);
		}
		return nullptr;
	}
	return &(m_blocks[index + BLOCKS_WIDE_Y]);
}

inline Block* Chunk::GetSouth(LocalIndex index)
{
	if ((index & LOCAL_Y_MASK) == 0x00)
	{
		if (m_southChunk)
		{
			LocalCoords blockOffsetCoords = GetLocalCoordsFromBlockIndex(index);
			blockOffsetCoords.y = BLOCKS_WIDE_Y - 1;
			LocalIndex neighborBlockIndex = GetBlockIndexFromLocalCoords(blockOffsetCoords);
			return m_southChunk->GetBlock(neighborBlockIndex);
		}
		return nullptr;
	}
	return &(m_blocks[index - BLOCKS_WIDE_Y]);
}

inline Block* Chunk::GetEast(LocalIndex index)
{
	if ((index & LOCAL_X_MASK) == LOCAL_X_MASK)
	{
		if (m_eastChunk)
		{
			LocalCoords blockOffsetCoords = GetLocalCoordsFromBlockIndex(index);
			blockOffsetCoords.x = 0;
			LocalIndex neighborBlockIndex = GetBlockIndexFromLocalCoords(blockOffsetCoords);
			return m_eastChunk->GetBlock(neighborBlockIndex);
		}
		return nullptr;
	}
	return &(m_blocks[index + 1]);
}

inline Block* Chunk::GetWest(LocalIndex index)
{
	if ((index & LOCAL_X_MASK) == 0x00)
	{
		if (m_westChunk)
		{
			LocalCoords blockOffsetCoords = GetLocalCoordsFromBlockIndex(index);
			blockOffsetCoords.x = BLOCKS_WIDE_X - 1;
			LocalIndex neighborBlockIndex = GetBlockIndexFromLocalCoords(blockOffsetCoords);
			return m_westChunk->GetBlock(neighborBlockIndex);
		}
		return nullptr;
	}
	return &(m_blocks[index - 1]);
}