#include "Game/Chunk.hpp"

//-----------------------------------------------------------------------------------
inline bool BlockInfo::IsOnEast() const
{
	return ((m_index & Chunk::LOCAL_X_MASK) == Chunk::LOCAL_X_MASK);
}

//-----------------------------------------------------------------------------------
inline bool BlockInfo::IsOnWest() const
{
	return ((m_index & Chunk::LOCAL_X_MASK) == 0x00);
}

//-----------------------------------------------------------------------------------
inline bool BlockInfo::IsOnNorth() const
{
	return ((m_index & Chunk::LOCAL_Y_MASK) == Chunk::LOCAL_Y_MASK);
}

//-----------------------------------------------------------------------------------
inline bool BlockInfo::IsOnSouth() const
{
	return ((m_index & Chunk::LOCAL_Y_MASK) == 0x00);
}

//-----------------------------------------------------------------------------------
inline bool BlockInfo::IsValid() const
{
	return m_index != INVALID_INDEX;
}

//-----------------------------------------------------------------------------------
inline Block* BlockInfo::GetBlock() const
{
	return IsValid() ? m_chunk->GetBlock(m_index) : nullptr;
}

//-----------------------------------------------------------------------------------
inline BlockInfo BlockInfo::GetAbove() const
{
	if ((m_index & Chunk::LOCAL_Z_MASK) == Chunk::LOCAL_Z_MASK)
	{
		return INVALID_BLOCK;
	}
	return BlockInfo(m_chunk, m_index + Chunk::BLOCKS_PER_LAYER);
}

//-----------------------------------------------------------------------------------
inline BlockInfo BlockInfo::GetBelow() const
{
	if ((m_index & Chunk::LOCAL_Z_MASK) == 0x00)
	{
		return INVALID_BLOCK;
	}
	return BlockInfo(m_chunk, m_index - Chunk::BLOCKS_PER_LAYER);
}

//-----------------------------------------------------------------------------------
inline BlockInfo BlockInfo::GetNorth() const
{
	if ((m_index & Chunk::LOCAL_Y_MASK) == Chunk::LOCAL_Y_MASK)
	{
		if (m_chunk->m_northChunk)
		{
			return BlockInfo(m_chunk->m_northChunk, m_index & ~Chunk::LOCAL_Y_MASK);
		}
		return INVALID_BLOCK;
	}
	return BlockInfo(m_chunk, m_index + Chunk::BLOCKS_WIDE_Y);
}

//-----------------------------------------------------------------------------------
inline BlockInfo BlockInfo::GetSouth() const
{
	if ((m_index & Chunk::LOCAL_Y_MASK) == 0x00)
	{
		if (m_chunk->m_southChunk)
		{
			return BlockInfo(m_chunk->m_southChunk, m_index | Chunk::LOCAL_Y_MASK);
		}
		return INVALID_BLOCK;
	}
	return BlockInfo(m_chunk, m_index - Chunk::BLOCKS_WIDE_Y);
}

//-----------------------------------------------------------------------------------
inline BlockInfo BlockInfo::GetEast() const
{
	if ((m_index & Chunk::LOCAL_X_MASK) == Chunk::LOCAL_X_MASK)
	{
		if (m_chunk->m_eastChunk)
		{
			return BlockInfo(m_chunk->m_eastChunk, m_index & ~Chunk::LOCAL_X_MASK);
		}
		return INVALID_BLOCK;
	}
	return BlockInfo(m_chunk, m_index + 1);
}

//-----------------------------------------------------------------------------------
inline BlockInfo BlockInfo::GetWest() const
{
	if ((m_index & Chunk::LOCAL_X_MASK) == 0x00)
	{
		if (m_chunk->m_westChunk)
		{
			return BlockInfo(m_chunk->m_westChunk, m_index | Chunk::LOCAL_X_MASK);
		}
		return INVALID_BLOCK;
	}
	return BlockInfo(m_chunk, m_index - 1);
}