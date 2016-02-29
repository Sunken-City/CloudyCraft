#pragma once
#include "Game/GameCommon.hpp"

class Block;
class Chunk;

//BlockInfo is a temporary data structure that allows access to a block and it's neighbors. 
//It's a flyweight "selector" that's unique for each block in a chunk. The accessor functions return
//newly constructed BlockInfo, as the information inside each one doesn't change.
class BlockInfo
{
public:
	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
	BlockInfo();
	BlockInfo(Chunk* chunkPointer, LocalIndex index);
	~BlockInfo();

	//ACCESSORS//////////////////////////////////////////////////////////////////////////
	inline Block* GetBlock() const;
	inline BlockInfo GetAbove() const;
	inline BlockInfo GetBelow() const;
	inline BlockInfo GetNorth() const;
	inline BlockInfo GetSouth() const;
	inline BlockInfo GetEast() const;
	inline BlockInfo GetWest() const;
	BlockInfo GetNeighbor(Direction direction) const;

	//QUERIES//////////////////////////////////////////////////////////////////////////
	inline bool IsOnEast() const;
	inline bool IsOnWest() const;
	inline bool IsOnNorth() const;
	inline bool IsOnSouth() const;
	inline bool IsValid() const;

	//FUNCTIONS//////////////////////////////////////////////////////////////////////////
	static void SetDirtyFlagAndAddToDirtyList(const BlockInfo& info);
	static Direction GetFaceDirectionFromNormal(const Vector3Int& normal);

	//CONSTANTS//////////////////////////////////////////////////////////////////////////
	static const int INVALID_INDEX = -1;
	static const BlockInfo INVALID_BLOCK;
	static const Direction directions[6];
	static const Direction s_oppositeDirections[6];
	static const Direction cardinalDirections[4];

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	Chunk* m_chunk;
	LocalIndex m_index;
};
 
#include "BlockInfo.inl"