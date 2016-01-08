#include "Game/BlockInfo.hpp"
#include "Game/Chunk.hpp"
#include "Game/World.hpp"

const BlockInfo BlockInfo::INVALID_BLOCK = BlockInfo(nullptr, INVALID_INDEX);

const Direction BlockInfo::cardinalDirections[] =
{
	Direction::NORTH,
	Direction::SOUTH,
	Direction::WEST,
	Direction::EAST
};


const Direction BlockInfo::directions[] =
{	
	Direction::ABOVE, 
	Direction::BELOW, 
	Direction::NORTH, 
	Direction::SOUTH, 
	Direction::WEST, 
	Direction::EAST 
};

BlockInfo::BlockInfo()
: m_chunk(nullptr)
, m_index(INVALID_INDEX)
{
}

BlockInfo::BlockInfo(Chunk* chunkPointer, LocalIndex index)
: m_chunk(chunkPointer)
, m_index(index)
{

}

BlockInfo::~BlockInfo()
{
}

Block* BlockInfo::GetBlock() const
{
	if (m_chunk == nullptr || m_index == INVALID_INDEX)
	{
		return nullptr;
	}
	return m_chunk->GetBlock(m_index);
}

BlockInfo BlockInfo::GetNeighbor(Direction direction) const
{
	switch (direction)
	{
	case Direction::ABOVE:
		return GetAbove();
	case Direction::BELOW:
		return GetBelow();
	case Direction::NORTH:
		return GetNorth();
	case Direction::SOUTH:
		return GetSouth();
	case Direction::EAST:
		return GetEast();
	case Direction::WEST:
		return GetWest();
	default:
		return INVALID_BLOCK;
	}
}

BlockInfo BlockInfo::GetAbove() const
{
	if ((m_index & Chunk::LOCAL_Z_MASK) == Chunk::LOCAL_Z_MASK)
	{
		return INVALID_BLOCK;
	}
	return BlockInfo(m_chunk, m_index + Chunk::BLOCKS_PER_LAYER);
}

BlockInfo BlockInfo::GetBelow() const
{
	if ((m_index & Chunk::LOCAL_Z_MASK) == 0x00)
	{
		return INVALID_BLOCK;
	}
	return BlockInfo(m_chunk, m_index - Chunk::BLOCKS_PER_LAYER);
}

BlockInfo BlockInfo::GetNorth() const
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

BlockInfo BlockInfo::GetSouth() const
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

BlockInfo BlockInfo::GetEast() const
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

BlockInfo BlockInfo::GetWest() const
{
	if ((m_index & Chunk::LOCAL_X_MASK) == 0x00)
	{
		if (m_chunk->m_westChunk)
		{
			return BlockInfo(m_chunk->m_westChunk, m_index | Chunk::LOCAL_X_MASK );
		}
		return INVALID_BLOCK;
	}
	return BlockInfo(m_chunk, m_index - 1);
}

bool BlockInfo::IsOnEast() const
{
	return ((m_index & Chunk::LOCAL_X_MASK) == Chunk::LOCAL_X_MASK);
}

bool BlockInfo::IsOnWest() const
{
	return ((m_index & Chunk::LOCAL_X_MASK) == 0x00);
}

bool BlockInfo::IsOnNorth() const
{
	return ((m_index & Chunk::LOCAL_Y_MASK) == Chunk::LOCAL_Y_MASK);
}

bool BlockInfo::IsOnSouth() const
{
	return ((m_index & Chunk::LOCAL_Y_MASK) == 0x00);
}

void BlockInfo::SetDirtyFlagAndAddToDirtyList(const BlockInfo& info)
{
	Block* block = info.GetBlock();
	if (block->IsDirty())
	{
		return;
	}
	block->SetDirty(true);
	info.m_chunk->m_world->m_dirtyBlocks.push_back(info);
}

bool BlockInfo::IsValid()
{
	return m_index != INVALID_INDEX;
}

