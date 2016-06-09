#include "Game/Chunk.hpp"
#include "Game/BlockInfo.hpp"

//-----------------------------------------------------------------------------------
inline WorldPosition Chunk::GetWorldMinsForBlockIndex(LocalIndex index) const
{
	LocalCoords coords = GetLocalCoordsFromBlockIndex(index);
	return WorldPosition((float)coords.x + m_bottomLeftCorner.x, (float)coords.y + m_bottomLeftCorner.y, (float)coords.z);
}

//-----------------------------------------------------------------------------------
inline LocalCoords Chunk::GetLocalCoordsFromBlockIndex(LocalIndex index) const
{
	return LocalCoords(index & LOCAL_X_MASK, (index & LOCAL_Y_MASK) >> CHUNK_BITS_X, index >> CHUNK_BITS_XY);
}

//-----------------------------------------------------------------------------------
inline LocalIndex Chunk::GetBlockIndexFromLocalCoords(const LocalCoords& coords) const
{
	return (coords.z << CHUNK_BITS_XY) + (coords.y << CHUNK_BITS_X) + coords.x;
}

//-----------------------------------------------------------------------------------
inline BlockInfo Chunk::GetBlockInfoFromLocalCoords(const LocalCoords& coords)
{
	return BlockInfo(this, GetBlockIndexFromLocalCoords(coords));
}

//-----------------------------------------------------------------------------------
inline Block* Chunk::GetBlock(LocalIndex index)
{
	if (index > BLOCKS_PER_CHUNK || index < 0)
	{
		return nullptr;
	}
	return &m_blocks[index];
}

//-----------------------------------------------------------------------------------
inline Block* Chunk::GetAbove(LocalIndex index)
{
	if ((index & LOCAL_Z_MASK) == LOCAL_Z_MASK)
	{
		return nullptr;
	}
	return &(m_blocks[index + BLOCKS_PER_LAYER]);
}

//-----------------------------------------------------------------------------------
inline Block* Chunk::GetBelow(LocalIndex index)
{
	if ((index & LOCAL_Z_MASK) == 0x00)
	{
		return nullptr;
	}
	return &(m_blocks[index - BLOCKS_PER_LAYER]);
}

//-----------------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------------
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