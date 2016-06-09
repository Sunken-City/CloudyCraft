#pragma once
#include <vector>
#include "Game/Block.hpp"
#include "Game/GameCommon.hpp"

class World;
class BlockInfo;

struct PortalBlockKey
{
	World* world;
	ChunkCoords chunkCoords;
	LocalIndex blockIndex;
};

//-----------------------------------------------------------------------------------
class Portal
{
public:
	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
	Portal();
	~Portal();

	//FUNCTIONS//////////////////////////////////////////////////////////////////////////
	static BlockInfo GetBlockInLinkedDimension(const BlockInfo& currentBlock);

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	World* sourceWorld;
	World* destinationWorld;
};