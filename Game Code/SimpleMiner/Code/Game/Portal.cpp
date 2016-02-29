#include "Game/Portal.hpp"
#include "Game/World.hpp"
#include "Game/BlockInfo.hpp"


Portal::Portal()
{

}

Portal::~Portal()
{

}

BlockInfo Portal::GetBlockInLinkedDimension(const BlockInfo& currentBlock)
{
	World* currentWorld = currentBlock.m_chunk->m_world;
	WorldCoords absolutePosition = currentBlock.m_chunk->GetWorldCoordsForBlockIndex(currentBlock.m_index);
	if (currentWorld == TheGame::instance->m_worlds[0])
	{
		return TheGame::instance->m_worlds[1]->GetBlockInfoFromWorldCoords(absolutePosition);
	}
	else
	{
		return TheGame::instance->m_worlds[0]->GetBlockInfoFromWorldCoords(absolutePosition);
	}
}
