#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Chunk.hpp"
#include "Game/BlockInfo.hpp"
#include "Game/Generator.hpp"
#include <map>
#include <set>
#include <deque>
#include <thread>

struct PrioritizedChunkCoords;
struct WorldChunkCoordsPair;

//GLOBALS//////////////////////////////////////////////////////////////////////////
//Primarily used for threading and profiling
extern std::set<PrioritizedChunkCoords> g_requestedChunkGenerationSet;
extern std::set<PrioritizedChunkCoords> g_requestedChunkLoadSet;
extern std::set<WorldChunkCoordsPair> g_chunksOnDiskSet;
extern std::deque<Chunk*> g_requestedChunkSaveDeque;
extern std::deque<Chunk*> g_readyToActivateChunksDeque;
extern ProfilingID g_generationProfiling;
extern ProfilingID g_loadingProfiling;
extern ProfilingID g_savingProfiling;
extern ProfilingID g_vaBuildingProfiling;
extern ProfilingID g_temporaryProfiling;

//STRUCTS//////////////////////////////////////////////////////////////////////////
struct WorldChunkCoordsPair
{
	WorldChunkCoordsPair(World* world, const ChunkCoords& coords) : world(world), chunkCoords(coords) {};
	inline bool operator<(const WorldChunkCoordsPair& rhs) const
	{
		if (this->world < rhs.world) return true;
		if (this->world > rhs.world) return false;
		return this->chunkCoords < rhs.chunkCoords;
	}

	World* world;
	ChunkCoords chunkCoords;
};

//-----------------------------------------------------------------------------------
struct PrioritizedChunkCoords
{
	PrioritizedChunkCoords() {};
	PrioritizedChunkCoords(World* world, const ChunkCoords& coords, float priorityValue) : world(world), chunkCoords(coords), prioritizedDistanceValue(priorityValue) {};
	inline bool operator<(const PrioritizedChunkCoords& rhs) const
	{
		//THIS MEANS THAT THE WORLD WITH THE LOWER POINTER VALUE HAS LOWER PRIORITY
		//FIXME
		if (this->world < rhs.world) return true;
		if (this->world > rhs.world) return false;
		if (this->prioritizedDistanceValue < rhs.prioritizedDistanceValue) return true;
		if (this->prioritizedDistanceValue > rhs.prioritizedDistanceValue) return false;
		return this->chunkCoords < rhs.chunkCoords;
	}

	World* world;
	ChunkCoords chunkCoords;
	float prioritizedDistanceValue;
};

//-----------------------------------------------------------------------------------
struct PrioritizedChunk
{
	PrioritizedChunk() {};
	PrioritizedChunk(Chunk* chunkPointer, float priorityValue) : chunk(chunkPointer), prioritizedDistanceValue(priorityValue) {};
	inline bool operator<(const PrioritizedChunk& rhs) const
	{
		return (this->prioritizedDistanceValue < rhs.prioritizedDistanceValue ? true : false);
	}

	Chunk* chunk;
	float prioritizedDistanceValue;
};

//-----------------------------------------------------------------------------------
struct RaycastResult3D
{
	bool didImpact;
	bool wasInsideBlockAlready;
	Vector3 impactPosition;
	Vector3Int impactTileCoords;
	float impactFraction;
	Vector3Int impactSurfaceNormal;
};


//THREADS//////////////////////////////////////////////////////////////////////////
void ChunkGenerationThreadMain();
void ChunkIOThreadMain();

class World
{
public:
	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
	World(int id, const RGBA& skyLight, const RGBA& skyColor, Generator* generator);
	~World();

	//FUNCTIONS//////////////////////////////////////////////////////////////////////////
	void Update(float deltaTime);
	void Render() const;
	void PlaceBlock();
	void DestroyBlock();
	void UpdateVertexArrays();
	void CreateRenderingOffsetList(Player* player);
	RaycastResult3D Raycast(const Vector3& start, const Vector3& end) const;

	//IO//////////////////////////////////////////////////////////////////////////
	static Chunk* LoadChunk(unsigned int worldID, ChunkCoords &chunkToGenerate);
	static void SaveChunk(Chunk* chunkToUnload);
	bool IsChunkOnDisk(ChunkCoords & chunkToGenerate);
	void FindAllChunksOnDisk();
	void AddToSaveQueue(Chunk* flushedChunk);

	//CONVERSIONS//////////////////////////////////////////////////////////////////////////
	inline ChunkCoords GetChunkCoordsFromWorldCoords(const WorldCoords& worldCoords) const;
	inline ChunkCoords GetChunkCoordsFromWorldPosition(const WorldPosition& worldPos) const;
	inline BlockInfo GetBlockInfoFromWorldCoords(const WorldCoords& worldCoords) const;
	ChunkCoords GetPlayerChunkCoords() const;
	ChunkCoords GetPlayerChunkCoords(Player* player) const;
	WorldPosition GetWorldPositionFromChunkCoords(const ChunkCoords& chunkCoords) const;
	Block* GetBlockFromWorldPosition(const WorldPosition& worldPos) const;
	Block* GetBlockFromWorldCoords(const WorldCoords& worldCoords) const;

	//CHUNK MANAGEMENT//////////////////////////////////////////////////////////////////////////
	bool GetHighestPriorityMissingChunk(PrioritizedChunkCoords& out_chunkCoords);
	bool GetFurthestUnneededChunk(ChunkCoords& out_chunkCoords);
	void ParseChunksInSquare(const AABB2 bounds);
	void HookUpChunkPointers(Chunk* chunkToHookUp);
	void UnhookChunkPointers(Chunk* chunkToUnhook);
	void RequestNeededChunks();
	void FlushUnnecessaryChunks();
	void RequestChunk(PrioritizedChunkCoords &chunkToGenerate);
	void PickUpCompletedChunks();
	int GetNumActiveChunks();
	float DistanceSquaredFromPlayerToChunk(ChunkCoords candidateChunkCoords);

	//LIGHTING//////////////////////////////////////////////////////////////////////////
	void UpdateLighting();
	void UpdateLightingForBlock(const BlockInfo& bi);
	void MarkAsLightingDirty(BlockInfo& bi);
	void SetBlockNeighborsDirty(const BlockInfo& info);
	RGBA GetIdealLightForBlock(const BlockInfo& bi);
	RGBA GetBrightestNeighbor(const BlockInfo& info);

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	unsigned int m_worldID;
	RGBA m_skyLight;
	RGBA m_skyColor;
	Generator* m_generator;
	std::deque<BlockInfo> m_dirtyBlocks;
	std::multiset<PrioritizedChunk> m_dirtyChunks;

private:
	static const int ACTIVE_RADIUS = 13;
	static const int FLUSH_RADIUS = 16;

	int m_chunkAddRemoveBalance;
	bool m_isChunkGenerationThreadDone;
	std::thread m_chunkGenerationThread;
	std::thread m_diskIOThread;
	std::map<ChunkCoords, PrioritizedChunkCoords> m_pendingRequests;
	std::map<ChunkCoords, Chunk*> m_activeChunks;
	std::vector<ChunkCoords> m_chunkRenderingOffsets;
};

#include "World.inl"