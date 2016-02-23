#include "Game/BlockInfo.hpp"
#include "Game/World.hpp"

const BlockInfo BlockInfo::INVALID_BLOCK = BlockInfo(nullptr, INVALID_INDEX);

//-----------------------------------------------------------------------------------
const Direction BlockInfo::s_oppositeDirections[] =
{
	Direction::BELOW,
	Direction::ABOVE,
	Direction::SOUTH,
	Direction::NORTH,
	Direction::WEST,
	Direction::EAST
};

//-----------------------------------------------------------------------------------
const Direction BlockInfo::cardinalDirections[] =
{
	Direction::NORTH,
	Direction::SOUTH,
	Direction::WEST,
	Direction::EAST
};

//-----------------------------------------------------------------------------------
const Direction BlockInfo::directions[] =
{	
	Direction::ABOVE, 
	Direction::BELOW, 
	Direction::NORTH, 
	Direction::SOUTH, 
	Direction::EAST,
	Direction::WEST
};

//-----------------------------------------------------------------------------------
BlockInfo::BlockInfo()
: m_chunk(nullptr)
, m_index(INVALID_INDEX)
{
}

//-----------------------------------------------------------------------------------
BlockInfo::BlockInfo(Chunk* chunkPointer, LocalIndex index)
: m_chunk(chunkPointer)
, m_index(index)
{

}

//-----------------------------------------------------------------------------------
BlockInfo::~BlockInfo()
{
}

//-----------------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------------
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

