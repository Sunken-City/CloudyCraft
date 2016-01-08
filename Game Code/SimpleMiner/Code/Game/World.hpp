#pragma once

#include "Game/GameCommon.hpp"
#include "Game/Chunk.hpp"
#include "Game/BlockInfo.hpp"
#include <map>
#include <set>
#include <deque>
#include <thread>

extern std::set<ChunkCoords> g_requestedChunkGenerationList;
extern std::set<ChunkCoords> g_requestedChunkLoadList;
extern std::set<ChunkCoords> g_chunksOnDisk;
extern std::deque<Chunk*> g_requestedChunkSaveList;
extern std::deque<Chunk*> g_readyToActivateChunks;
extern ProfilingID g_generationProfiling;
extern ProfilingID g_loadingProfiling;
extern ProfilingID g_savingProfiling;
extern ProfilingID g_vaBuildingProfiling;
extern ProfilingID g_temporaryProfiling;

struct RaycastResult3D
{
	bool didImpact;
	Vector3 impactPosition;
	Vector3Int impactTileCoords;
	float impactFraction;
	Vector3Int impactSurfaceNormal;
};

void ChunkGenerationThreadMain();
void ChunkIOThreadMain();

class World
{
public:
	World();
	~World();
	void Update(float deltaTime);
	void Render() const;
	void PlaceBlock();
	void DestroyBlock();
	void RequestNeededChunks();
	void FlushUnnecessaryChunks();
	void RequestChunk(ChunkCoords &chunkToGenerate);

	// IO //////////////////////////////////////////////////////////////////////////
	static Chunk* LoadChunk(ChunkCoords &chunkToGenerate);
	static void SaveChunk(Chunk* chunkToUnload);
	void FindAllChunksOnDisk();
	bool IsChunkOnDisk(ChunkCoords & chunkToGenerate);

	bool GetHighestPriorityMissingChunk(ChunkCoords& out_chunkCoords);
	bool GetFurthestUnneededChunk(ChunkCoords& out_chunkCoords);
	void HookUpChunkPointers(Chunk* chunkToHookUp);
	void UnhookChunkPointers(Chunk* chunkToUnhook);
	ChunkCoords GetPlayerChunkCoords() const;
	ChunkCoords GetPlayerChunkCoords(Player* player) const;
	WorldPosition GetWorldPositionFromChunkCoords(const ChunkCoords& chunkCoords) const;
	inline ChunkCoords GetChunkCoordsFromWorldCoords(const WorldCoords& worldCoords) const;
	inline ChunkCoords GetChunkCoordsFromWorldPosition(const WorldPosition& worldPos) const;
	inline BlockInfo GetBlockInfoFromWorldCoords(const WorldCoords& worldCoords) const;
	Block* GetBlockFromWorldPosition(const WorldPosition& worldPos) const;
	Block* GetBlockFromWorldCoords(const WorldCoords& worldCoords) const;
	RaycastResult3D Raycast(const Vector3& start, const Vector3& end) const;
	void UpdateLighting();
	void UpdateLightingForBlock(const BlockInfo& bi);
	void MarkAsLightingDirty(BlockInfo& bi);
	void SetBlockNeighborsDirty(const BlockInfo& info);
	void UpdateVertexArrays();
	uchar GetIdealRedLightForBlock(const BlockInfo& bi);
	uchar GetIdealGreenLightForBlock(const BlockInfo& bi);
	uchar GetIdealBlueLightForBlock(const BlockInfo& bi);
	uchar GetBrightestRedNeighbor(const BlockInfo& info);
	uchar GetBrightestGreenNeighbor(const BlockInfo& info);
	uchar GetBrightestBlueNeighbor(const BlockInfo& info);
	int GetNumActiveChunks();
	void ParseChunksInSquare(const AABB2 bounds);
	void CreateRenderingOffsetList(Player* player);
	void AddToSaveQueue(Chunk* flushedChunk);
	void PickUpCompletedChunks();

	static const unsigned int SKY_LIGHT = 0xDDEEFF00; //Sunset 0xFF990000   Vaporwave 0xFF819C00
	std::deque<BlockInfo> m_dirtyBlocks;
private:
	static const int ACTIVE_RADIUS = 13;
	static const int FLUSH_RADIUS = 16;

	int m_chunkAddRemoveBalance;
	bool m_isChunkGenerationThreadDone;
	std::thread m_chunkGenerationThread;
	std::thread m_diskIOThread;
	std::map<ChunkCoords, Chunk*> m_activeChunks;
	std::deque<Chunk*> m_dirtyChunks;
	std::vector<ChunkCoords> m_chunkRenderingOffsets;
};

inline ChunkCoords World::GetChunkCoordsFromWorldPosition(const WorldPosition& worldPos) const
{
	return ChunkCoords(static_cast<int>(floor(worldPos.x)), static_cast<int>(floor(worldPos.y)));
}

inline ChunkCoords World::GetChunkCoordsFromWorldCoords(const WorldCoords& worldCoords) const
{
	return ChunkCoords(static_cast<int>(floor((float)worldCoords.x / (float)Chunk::BLOCKS_WIDE_X)), static_cast<int>(floor((float)worldCoords.y / (float)Chunk::BLOCKS_WIDE_Y)));
}

inline BlockInfo World::GetBlockInfoFromWorldCoords(const WorldCoords& worldCoords) const
{
	ChunkCoords chunkPosition = GetChunkCoordsFromWorldCoords(worldCoords);
	auto chunkData = m_activeChunks.find(chunkPosition);
	if (chunkData == m_activeChunks.end())
	{
		return BlockInfo::INVALID_BLOCK;
	}
	Chunk* chunk = chunkData->second;
	LocalIndex index = chunk->GetBlockIndexFromWorldCoords(worldCoords);
	return BlockInfo(chunk, index);
}
