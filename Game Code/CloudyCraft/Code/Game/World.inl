#include "Game/World.hpp"

//-----------------------------------------------------------------------------------
inline ChunkCoords World::GetChunkCoordsFromWorldPosition(const WorldPosition& worldPos) const
{
	return ChunkCoords(static_cast<int>(floor(worldPos.x)), static_cast<int>(floor(worldPos.y)));
}

//-----------------------------------------------------------------------------------
inline ChunkCoords World::GetChunkCoordsFromWorldCoords(const WorldCoords& worldCoords) const
{
	return ChunkCoords(static_cast<int>(floor((float)worldCoords.x / (float)Chunk::BLOCKS_WIDE_X)), static_cast<int>(floor((float)worldCoords.y / (float)Chunk::BLOCKS_WIDE_Y)));
}

//-----------------------------------------------------------------------------------
inline BlockInfo World::GetBlockInfoFromWorldCoords(const WorldCoords& worldCoords) const
{
	ChunkCoords chunkPosition = GetChunkCoordsFromWorldCoords(worldCoords);
	auto chunkData = m_activeChunks.find(chunkPosition);
	if (chunkData == m_activeChunks.end())
	{
		return BlockInfo::INVALID_BLOCKINFO;
	}
	Chunk* chunk = chunkData->second;
	LocalIndex index = chunk->GetBlockIndexFromWorldCoords(worldCoords);
	return BlockInfo(chunk, index);
}
