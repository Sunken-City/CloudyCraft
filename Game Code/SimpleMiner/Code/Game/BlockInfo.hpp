#pragma once
#include "Game/GameCommon.hpp"

class Block;
class Chunk;

enum class Direction
{
	ABOVE = 0,
	BELOW,
	NORTH,
	SOUTH,
	EAST,
	WEST,
	NUM_DIRECTIONS
};

class BlockInfo
{
public:
	BlockInfo();
	BlockInfo(Chunk* chunkPointer, LocalIndex index);
	~BlockInfo();

	Block* GetBlock() const;
	BlockInfo GetNeighbor(Direction direction) const;
	BlockInfo GetAbove() const;
	BlockInfo GetBelow() const;
	BlockInfo GetNorth() const;
	BlockInfo GetSouth() const;
	BlockInfo GetEast() const;
	BlockInfo GetWest() const; 	
	bool IsOnEast() const;
	bool IsOnWest() const;
	bool IsOnNorth() const;
	bool IsOnSouth() const;
	bool IsValid();
	static void SetDirtyFlagAndAddToDirtyList(const BlockInfo& info);

	static const int INVALID_INDEX = -1;
	static const BlockInfo INVALID_BLOCK;
	static const Direction directions[6];
	static const Direction cardinalDirections[4];
	Chunk* m_chunk;
	LocalIndex m_index;
};
