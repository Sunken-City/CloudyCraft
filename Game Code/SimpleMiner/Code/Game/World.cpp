#include "Game/World.hpp"
#include "Game/BlockDefinition.h"
#include "Game/TheGame.hpp"
#include "Game/Camera3D.hpp"
#include "Game/Player.hpp"
#include "Game/Generator.hpp"
#include "Engine/Input/InputOutputUtils.hpp"
#include "Engine/Renderer/Face.hpp"
#include "Engine/Renderer/Vertex.hpp"
#include "Engine/Time/Time.hpp"
#include <algorithm>
#include <regex>
#include <string.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <thread>

std::vector<Vertex_PCT> g_debugPoints;
std::set<PrioritizedChunkCoords> g_requestedChunkGenerationSet;
std::set<PrioritizedChunkCoords> g_requestedChunkLoadSet;
std::set<WorldChunkCoordsPair> g_chunksOnDiskSet;
std::deque<Chunk*> g_requestedChunkSaveDeque;
std::deque<Chunk*> g_readyToActivateChunksDeque; 
ProfilingID g_generationProfiling;
ProfilingID g_loadingProfiling;
ProfilingID g_savingProfiling;
ProfilingID g_vaBuildingProfiling;
ProfilingID g_temporaryProfiling;
extern CRITICAL_SECTION g_chunkListsCriticalSection;
extern CRITICAL_SECTION g_diskIOCriticalSection;

//-----------------------------------------------------------------------------------
World::World(int id, const RGBA& skyLight, const RGBA& skyColor, Generator* generator)
	: m_worldID(id)
	, m_chunkAddRemoveBalance(0)
	, m_isChunkGenerationThreadDone(false)
	, m_chunkGenerationThread(ChunkGenerationThreadMain)
	, m_diskIOThread(ChunkIOThreadMain)
	, m_skyLight(skyLight) //Daylight 0xDDEEFF00  Sunset 0xFF990000  Vaporwave 0xFF819C00
	, m_skyColor(skyColor)
	, m_generator(generator)
{
	BlockDefinition::Initialize();
	FindAllChunksOnDisk();

	g_generationProfiling = RegisterProfilingChannel();
	g_loadingProfiling = RegisterProfilingChannel();
	g_savingProfiling = RegisterProfilingChannel();
	g_vaBuildingProfiling = RegisterProfilingChannel();
	g_temporaryProfiling = RegisterProfilingChannel();

	m_chunkGenerationThread.detach();
	m_diskIOThread.detach();
}

//-----------------------------------------------------------------------------------
World::~World()
{
	//Wait for the other thread to finish shutting down, then continue.
	if (m_chunkGenerationThread.joinable())
	{
		m_chunkGenerationThread.join();
	}
	if (m_diskIOThread.joinable())
	{
		m_diskIOThread.join();
	}
	for (auto chunkToFlushPair : m_activeChunks)
	{
		Chunk* flushedChunk = chunkToFlushPair.second;
		SaveChunk(flushedChunk);
		delete(flushedChunk);
	}
}

//-----------------------------------------------------------------------------------
void World::Update(float deltaTime)
{
	RequestNeededChunks();
	PickUpCompletedChunks();
	FlushUnnecessaryChunks();
	for (auto currentChunkPair : m_activeChunks)
	{
		Chunk* currentChunk = currentChunkPair.second;
		currentChunk->Update(deltaTime);
	}
	UpdateLighting();
	UpdateVertexArrays();
}

//-----------------------------------------------------------------------------------
void World::UpdateVertexArrays()
{
	static double timeLastShuffled;
	int numberOfChunksWaitingForVAUpdate = m_dirtyChunks.size();
	double timeInSeconds = GetCurrentTimeSeconds();
	DebuggerPrintf("[%i] World [%i]: Number of chunks awaiting VA Updates: %i\n", g_frameNumber, m_worldID, numberOfChunksWaitingForVAUpdate);
	if (numberOfChunksWaitingForVAUpdate > 40 && (timeInSeconds - timeLastShuffled) > 0.5)
	{
		timeLastShuffled = timeInSeconds;
		DebuggerPrintf("[%i] World [%i]: Recalculating chunk queue...\n", g_frameNumber, m_worldID);
		Chunk** chunkPointerHolder = new Chunk*[numberOfChunksWaitingForVAUpdate];
		int currentIndex = 0;
		for (PrioritizedChunk chunkPair : m_dirtyChunks)
		{
			chunkPointerHolder[currentIndex++] = chunkPair.chunk;
		}
		m_dirtyChunks.clear();
		for (int i = 0; i < numberOfChunksWaitingForVAUpdate; i++)
		{
			m_dirtyChunks.emplace(chunkPointerHolder[i], chunkPointerHolder[i]->m_world->DistanceSquaredFromPlayerToChunk(chunkPointerHolder[i]->m_chunkPosition));
		}
		delete [] chunkPointerHolder;
	}
	if (!m_dirtyChunks.empty())
	{
		Chunk* chunkToUpdate = m_dirtyChunks.begin()->chunk;
		chunkToUpdate->GenerateVertexArray();
		m_dirtyChunks.erase(m_dirtyChunks.begin());
	}
}

//-----------------------------------------------------------------------------------
void World::RequestNeededChunks()
{
	PrioritizedChunkCoords chunkToGenerate(this, ChunkCoords(0,0), 9999999.0f);
	bool shouldGenerateChunk = GetHighestPriorityMissingChunk(chunkToGenerate);
	if (shouldGenerateChunk)
	{
		RequestChunk(chunkToGenerate);
	}
}

//-----------------------------------------------------------------------------------
void World::FlushUnnecessaryChunks()
{
	ChunkCoords chunkToFlush(0, 0);
	bool shouldFlushChunk = GetFurthestUnneededChunk(chunkToFlush);

	if (shouldFlushChunk)
	{
		Chunk* flushedChunk = m_activeChunks[chunkToFlush];
		AddToSaveQueue(flushedChunk);
		UnhookChunkPointers(m_activeChunks[chunkToFlush]);
		m_chunkAddRemoveBalance--;
		m_activeChunks.erase(chunkToFlush);
	}
}

//-----------------------------------------------------------------------------------
void World::FindAllChunksOnDisk()
{
	std::vector<std::string>* fileNames = GetFileNamesInFolder(Stringf("Data\\SaveData\\Save0\\World%i\\*", m_worldID));
	std::regex chunkFile ("-?[0-9]+,-?[0-9]+.*\\.chunk");
	for (std::string fileName : *fileNames)
	{
		if (regex_match(fileName, chunkFile))
		{
			ChunkCoords fileCoords;
			const int fileNameLength = fileName.length() + 1;
			char *fileCStr = new char[fileNameLength];
			strcpy_s(fileCStr, fileNameLength, fileName.c_str());
			char* context = nullptr;
			char* firstNumber = strtok_s(fileCStr, ",.", &context);
			fileCoords.x = atoi(firstNumber);
			char* secondNumber = strtok_s(NULL, ",.", &context);
			fileCoords.y = atoi(secondNumber);
			g_chunksOnDiskSet.emplace(this, fileCoords);
			delete [] fileCStr;
		}
	}
	delete fileNames;
}

//-----------------------------------------------------------------------------------
bool World::IsChunkOnDisk(ChunkCoords & chunkToGenerate)
{
	WorldChunkCoordsPair chunkCoordsInWorld (this, chunkToGenerate);
	return (g_chunksOnDiskSet.find(chunkCoordsInWorld) != g_chunksOnDiskSet.end());
}

//-----------------------------------------------------------------------------------
void World::RequestChunk(PrioritizedChunkCoords &prioritizedChunkCoordsToGenerate)
{
	bool isOnDisk = IsChunkOnDisk(prioritizedChunkCoordsToGenerate.chunkCoords);
	if (isOnDisk)
	{
		EnterCriticalSection(&g_diskIOCriticalSection);
		{
			g_requestedChunkLoadSet.insert(prioritizedChunkCoordsToGenerate);
		}
		LeaveCriticalSection(&g_diskIOCriticalSection);
	}
	else
	{
		EnterCriticalSection(&g_chunkListsCriticalSection);
		{
			g_requestedChunkGenerationSet.insert(prioritizedChunkCoordsToGenerate);
		}
		LeaveCriticalSection(&g_chunkListsCriticalSection);
	}
	m_pendingRequests[prioritizedChunkCoordsToGenerate.chunkCoords] = prioritizedChunkCoordsToGenerate;
}

//-----------------------------------------------------------------------------------
Chunk* World::LoadChunk(unsigned int worldID, ChunkCoords &chunkToGenerate)
{
	StartTiming(g_loadingProfiling);
	std::vector<uchar> chunkData;
	Chunk* loadedChunk = nullptr;

	bool fileLoaded = LoadBufferFromBinaryFile(chunkData, Stringf("Data\\SaveData\\Save0\\World%i\\%i,%i.chunk", worldID, chunkToGenerate.x, chunkToGenerate.y));
	if (fileLoaded)
	{
		loadedChunk = new Chunk(chunkToGenerate, chunkData, TheGame::instance->m_worlds[worldID]);
	}
	EndTiming(g_loadingProfiling);
	return loadedChunk;
}

//-----------------------------------------------------------------------------------
void World::Render() const
{
	ChunkCoords playerPos = GetPlayerChunkCoords();
	for (ChunkCoords offset : m_chunkRenderingOffsets)
	{
		ChunkCoords currentChunkCoords = playerPos + offset;
		Chunk* currentChunk = nullptr;
		auto chunkIter = m_activeChunks.find(currentChunkCoords);
		if (chunkIter != m_activeChunks.end())
		{
			currentChunk = chunkIter->second;
			if (currentChunk->IsInFrustum(TheGame::instance->m_playerCamera->GetForwardXYZ(), TheGame::instance->m_playerCamera->m_position))
			{
				currentChunk->Render();
			}
		}
	}
}

//-----------------------------------------------------------------------------------
void World::PlaceBlock()
{
	Player* player = TheGame::instance->m_player;
	Chunk* currentChunk = m_activeChunks[GetChunkCoordsFromWorldCoords(player->m_raycastResult.impactTileCoords)];
	WorldCoords blockToPlace = player->m_raycastResult.impactTileCoords + player->m_raycastResult.impactSurfaceNormal;
	BlockInfo info = GetBlockInfoFromWorldCoords(blockToPlace);
	Block* block = info.GetBlock();
	block->m_type = player->m_heldBlock;
	BlockDefinition* definition = BlockDefinition::GetDefinition(player->m_heldBlock);
	AudioSystem::instance->PlaySound(definition->m_placeSound);
	currentChunk->SetHighPriorityChunkDirtyAndAddToDirtyList();
	BlockInfo::SetDirtyFlagAndAddToDirtyList(info);
	if (block->IsEdgeBlock())
	{
		BlockInfo east = info.GetEast();
		BlockInfo west = info.GetWest();
		BlockInfo north = info.GetNorth();
		BlockInfo south = info.GetSouth();
		if (info.IsOnEast() && east.m_chunk)
		{
			east.m_chunk->DirtyAndAddToDirtyList();
		}
		else if (info.IsOnWest() && west.m_chunk)
		{
			west.m_chunk->DirtyAndAddToDirtyList();
		}

		if (info.IsOnNorth() && north.m_chunk)
		{
			north.m_chunk->DirtyAndAddToDirtyList();
		}
		else if (info.IsOnSouth() && south.m_chunk)
		{
			south.m_chunk->DirtyAndAddToDirtyList();
		}
	}
	if (info.GetBlock()->IsSky())
	{
		block->SetSky(false);
		info = info.GetBelow();
		while (info.m_index != BlockInfo::INVALID_INDEX)
		{
			Block* currentBlock = info.GetBlock();
			uchar blockType = currentBlock->m_type;
			if (BlockDefinition::GetDefinition(blockType)->m_isOpaque)
			{
				info = BlockInfo::INVALID_BLOCK;
			}
			else
			{
				currentBlock->SetSky(false);
				BlockInfo::SetDirtyFlagAndAddToDirtyList(info);
				info = info.GetBelow();
			}
		}
	}
}

//-----------------------------------------------------------------------------------
void World::DestroyBlock()
{
	Player* player = TheGame::instance->m_player;
	BlockInfo info = GetBlockInfoFromWorldCoords(player->m_raycastResult.impactTileCoords);
	if (!info.IsValid())
	{
		return;
	}
	Block* block = info.GetBlock();
	BlockDefinition* definition = info.GetBlock()->GetDefinition();
	AudioSystem::instance->PlaySound(definition->m_brokenSound);
	block->m_type = BlockType::AIR;
	//This chunk is NOT high priority because we want the edge chunk to get updated first.
	//If we don't, we'll see a gap in the world before the other chunk's VA gets updated. This chunk is next in line regardless.
	info.m_chunk->DirtyAndAddToDirtyList();
	BlockInfo::SetDirtyFlagAndAddToDirtyList(info);
	//Update VA's if we were on a boundary
	if (block->IsEdgeBlock())
	{
		BlockInfo east = info.GetEast();
		BlockInfo west = info.GetWest();
		BlockInfo north = info.GetNorth();
		BlockInfo south = info.GetSouth();
		if (info.IsOnEast() && east.m_chunk)
		{
			east.m_chunk->SetHighPriorityChunkDirtyAndAddToDirtyList();
		}
		else if (info.IsOnWest() && west.m_chunk)
		{
			west.m_chunk->SetHighPriorityChunkDirtyAndAddToDirtyList();
		}

		if (info.IsOnNorth() && north.m_chunk)
		{
			north.m_chunk->SetHighPriorityChunkDirtyAndAddToDirtyList();
		}
		else if (info.IsOnSouth() && south.m_chunk)
		{
			south.m_chunk->SetHighPriorityChunkDirtyAndAddToDirtyList();
		}
	}
	if (info.GetAbove().GetBlock()->IsSky())
	{
		while (info.m_index != BlockInfo::INVALID_INDEX)
		{
			Block* currentBlock = info.GetBlock();
			uchar blockType = currentBlock->m_type;
			BlockDefinition* blockDef = BlockDefinition::GetDefinition(blockType);
			if (blockDef->m_isOpaque)
			{
				info = BlockInfo::INVALID_BLOCK;
			}
			else
			{
				if (block->m_type != BlockType::WATER)
				{
					currentBlock->SetSky(true);
				}
				BlockInfo::SetDirtyFlagAndAddToDirtyList(info);
				info = info.GetBelow();
			}
		}
	}

}

//-----------------------------------------------------------------------------------
bool World::GetHighestPriorityMissingChunk(PrioritizedChunkCoords& out_chunkCoords)
{
	const float radiusBlocks = ACTIVE_RADIUS * Chunk::BLOCKS_WIDE_X;
	const float radiusSquared = radiusBlocks * radiusBlocks;
	bool shouldGenerate = false;
	ChunkCoords mostUrgent;
	float distToMostUrgent = 9999999.0f;
	
	ChunkCoords playerChunk = GetPlayerChunkCoords();
	for (int x = playerChunk.x - ACTIVE_RADIUS; x < playerChunk.x + ACTIVE_RADIUS; x++)
	{
		for (int y = playerChunk.y - ACTIVE_RADIUS; y < playerChunk.y + ACTIVE_RADIUS; y++)
		{
			ChunkCoords candidateChunkCoords(x, y);
			float distToChunk = DistanceSquaredFromPlayerToChunk(candidateChunkCoords);

			if (distToChunk < radiusSquared && m_activeChunks.find(candidateChunkCoords) == m_activeChunks.end() && m_pendingRequests.find(candidateChunkCoords) == m_pendingRequests.end())
			{
				if (distToChunk < distToMostUrgent)
				{
					mostUrgent = candidateChunkCoords;
					distToMostUrgent = distToChunk;
					shouldGenerate = true;
				}
			}
		}
	}
	out_chunkCoords.chunkCoords = mostUrgent;
	out_chunkCoords.prioritizedDistanceValue = distToMostUrgent;
	return shouldGenerate;
}

//-----------------------------------------------------------------------------------
float World::DistanceSquaredFromPlayerToChunk(ChunkCoords candidateChunkCoords)
{
	WorldPosition worldPosCandidateChunk = GetWorldPositionFromChunkCoords(candidateChunkCoords);
	WorldPosition adjustedPlayerPosition = TheGame::instance->m_playerCamera->m_position;
	adjustedPlayerPosition.z = 0.0f;
	return MathUtils::CalcDistSquaredBetweenPoints(adjustedPlayerPosition, worldPosCandidateChunk);
}

//-----------------------------------------------------------------------------------
bool World::GetFurthestUnneededChunk(ChunkCoords& out_chunkCoords)
{
	const float radiusBlocks = ACTIVE_RADIUS * Chunk::BLOCKS_WIDE_X;
	const float radiusSquared = radiusBlocks * radiusBlocks;
	bool shouldRemove = false;
	ChunkCoords mostUrgent;
	float distToMostUrgent = 0.0f;
	ChunkCoords playerChunk = GetPlayerChunkCoords();
	for (int x = playerChunk.x - FLUSH_RADIUS; x < playerChunk.x + FLUSH_RADIUS; x++)
	{
		for (int y = playerChunk.y - FLUSH_RADIUS; y < playerChunk.y + FLUSH_RADIUS; y++)
		{
			if (abs(y) > abs(playerChunk.y - ACTIVE_RADIUS) && abs(y) < abs(playerChunk.y + ACTIVE_RADIUS)
				&& (abs(x) > abs(playerChunk.x - ACTIVE_RADIUS) && abs(x) < abs(playerChunk.x + ACTIVE_RADIUS)))
			{
				continue;
			}
			ChunkCoords candidateChunkCoords(x, y);
			WorldPosition worldPosCandidateChunk = GetWorldPositionFromChunkCoords(candidateChunkCoords);
			WorldPosition adjustedPlayerPosition = TheGame::instance->m_playerCamera->m_position;
			adjustedPlayerPosition.z = 0.0f;
			float distToChunk = MathUtils::CalcDistSquaredBetweenPoints(adjustedPlayerPosition, worldPosCandidateChunk);
			if (distToChunk > radiusSquared && m_activeChunks.find(candidateChunkCoords) != m_activeChunks.end())
			{
				if (distToChunk > distToMostUrgent)
				{
					mostUrgent = candidateChunkCoords;
					distToMostUrgent = distToChunk;
					shouldRemove = true;
				}
			}
		}
	}
	out_chunkCoords = mostUrgent;
	return shouldRemove;
}

//-----------------------------------------------------------------------------------
void World::SaveChunk(Chunk* chunkToUnload)
{
	StartTiming(g_savingProfiling);
	std::vector<uchar> chunkData;
	chunkToUnload->GenerateSaveData(chunkData);
	SaveBufferToBinaryFile(chunkData, Stringf("Data\\SaveData\\Save0\\World%i\\%i,%i.chunk", chunkToUnload->m_world->m_worldID, chunkToUnload->m_chunkPosition.x, chunkToUnload->m_chunkPosition.y));
	g_chunksOnDiskSet.emplace(chunkToUnload->m_world, chunkToUnload->m_chunkPosition);
	EndTiming(g_savingProfiling);
}

//-----------------------------------------------------------------------------------
void World::HookUpChunkPointers(Chunk* chunkToHookUp)
{
	ChunkCoords position = chunkToHookUp->m_chunkPosition;
	ChunkCoords eastChunkPos = position + Vector2Int(1, 0);
	ChunkCoords westChunkPos = position + Vector2Int(-1, 0);
	ChunkCoords northChunkPos = position + Vector2Int(0, 1);
	ChunkCoords southChunkPos = position + Vector2Int(0, -1);
	auto eastChunk = m_activeChunks.find(eastChunkPos);
	if (eastChunk != m_activeChunks.end())
	{
		Chunk* existingNeighborChunk = eastChunk->second;
		chunkToHookUp->m_eastChunk = existingNeighborChunk;
		existingNeighborChunk->m_westChunk = chunkToHookUp;
		existingNeighborChunk->FlagEdgesAsDirtyLighting(WEST);
		existingNeighborChunk->DirtyAndAddToDirtyList();
	}
	auto westChunk = m_activeChunks.find(westChunkPos);
	if (westChunk != m_activeChunks.end())
	{
		chunkToHookUp->m_westChunk = westChunk->second;
		Chunk* existingNeighborChunk = westChunk->second;
		existingNeighborChunk->m_eastChunk = chunkToHookUp;
		existingNeighborChunk->FlagEdgesAsDirtyLighting(EAST);
		existingNeighborChunk->DirtyAndAddToDirtyList();
	}
	auto northChunk = m_activeChunks.find(northChunkPos);
	if (northChunk != m_activeChunks.end())
	{
		chunkToHookUp->m_northChunk = northChunk->second;
		Chunk* existingNeighborChunk = northChunk->second;
		existingNeighborChunk->m_southChunk = chunkToHookUp;
		existingNeighborChunk->FlagEdgesAsDirtyLighting(SOUTH);
		existingNeighborChunk->DirtyAndAddToDirtyList();
	}
	auto southChunk = m_activeChunks.find(southChunkPos);
	if (southChunk != m_activeChunks.end())
	{
		chunkToHookUp->m_southChunk = southChunk->second;
		Chunk* existingNeighborChunk = southChunk->second;
		existingNeighborChunk->m_northChunk = chunkToHookUp;
		existingNeighborChunk->FlagEdgesAsDirtyLighting(NORTH);
		existingNeighborChunk->DirtyAndAddToDirtyList();
	}
}

//-----------------------------------------------------------------------------------
void World::UnhookChunkPointers(Chunk* chunkToUnhook)
{
	ChunkCoords position = chunkToUnhook->m_chunkPosition;
	ChunkCoords eastChunkPos = position + Vector2Int(1, 0);
	ChunkCoords westChunkPos = position + Vector2Int(-1, 0);
	ChunkCoords northChunkPos = position + Vector2Int(0, 1);
	ChunkCoords southChunkPos = position + Vector2Int(0, -1);
	auto eastChunk = m_activeChunks.find(eastChunkPos);
	if (eastChunk != m_activeChunks.end())
	{
		Chunk* existingNeighborChunk = eastChunk->second;
		existingNeighborChunk->m_westChunk = nullptr;
	}
	auto westChunk = m_activeChunks.find(westChunkPos);
	if (westChunk != m_activeChunks.end())
	{
		Chunk* existingNeighborChunk = westChunk->second;
		existingNeighborChunk->m_eastChunk = nullptr;
	}
	auto northChunk = m_activeChunks.find(northChunkPos);
	if (northChunk != m_activeChunks.end())
	{
		Chunk* existingNeighborChunk = northChunk->second;
		existingNeighborChunk->m_southChunk = nullptr;
	}
	auto southChunk = m_activeChunks.find(southChunkPos);
	if (southChunk != m_activeChunks.end())
	{
		Chunk* existingNeighborChunk = southChunk->second;
		existingNeighborChunk->m_northChunk = nullptr;
	}
}

//-----------------------------------------------------------------------------------
ChunkCoords World::GetPlayerChunkCoords() const
{
	WorldPosition playerPosition = TheGame::instance->m_player->m_position;
	return ChunkCoords(static_cast<int>(floor(playerPosition.x / Chunk::BLOCKS_WIDE_X)), static_cast<int>(floor(playerPosition.y / Chunk::BLOCKS_WIDE_X)));
}

//-----------------------------------------------------------------------------------
ChunkCoords World::GetPlayerChunkCoords(Player* player) const
{
	WorldPosition playerPosition = player->m_position;
	return ChunkCoords(static_cast<int>(floor(playerPosition.x / Chunk::BLOCKS_WIDE_X)), static_cast<int>(floor(playerPosition.y / Chunk::BLOCKS_WIDE_X)));
}

//-----------------------------------------------------------------------------------
WorldPosition World::GetWorldPositionFromChunkCoords(const ChunkCoords& chunkCoords) const
{
	const float halfX = Chunk::BLOCKS_WIDE_X / 2.0f;
	const float halfY = Chunk::BLOCKS_WIDE_Y / 2.0f;
	float worldX = (chunkCoords.x * Chunk::BLOCKS_WIDE_X) + halfX;
	float worldY = (chunkCoords.y * Chunk::BLOCKS_WIDE_Y) + halfY;
	return WorldPosition(worldX, worldY, 0.0f);
}

//-----------------------------------------------------------------------------------
Block* World::GetBlockFromWorldPosition(const WorldPosition& worldPos) const
{
	return GetBlockFromWorldCoords(WorldCoords(static_cast<int>(floor(worldPos.x)), static_cast<int>(floor(worldPos.y)), static_cast<int>(floor(worldPos.z))));
}

//-----------------------------------------------------------------------------------
Block* World::GetBlockFromWorldCoords(const WorldCoords& worldCoords) const
{
	ChunkCoords chunkPosition = GetChunkCoordsFromWorldCoords(worldCoords);
	auto chunkData = m_activeChunks.find(chunkPosition);
	if (chunkData == m_activeChunks.end())
	{
		return nullptr;
	}
	Chunk* chunk = chunkData->second;
	LocalIndex index = chunk->GetBlockIndexFromWorldCoords(worldCoords);
	return chunk->GetBlock(index);
}

//-----------------------------------------------------------------------------------
RaycastResult3D World::Raycast(const Vector3& start, const Vector3& end) const
{
	RaycastResult3D result;
	result.didImpact = false;
	result.wasInsideBlockAlready = false;
	result.impactFraction = 1.0f;
	result.impactPosition = Vector3::ZERO;
	result.impactSurfaceNormal = Vector3Int(0, 0, 0);

	Block* startBlock = GetBlockFromWorldPosition(start);
	if (!startBlock)
	{
		return result;
	}
	uchar blockType = startBlock->m_type;
	WorldCoords rayPosition = WorldCoords(static_cast<int>(floor(start.x)), static_cast<int>(floor(start.y)), static_cast<int>(floor(start.z)));
	if (BlockDefinition::GetDefinition(blockType)->IsSolid())
	{
		result.didImpact = true;
		result.wasInsideBlockAlready = true;
		result.impactFraction = 0.0f;
		result.impactPosition = start;
		result.impactTileCoords = rayPosition;
		result.impactSurfaceNormal = Vector3Int(0, 0, 0);
		if (start.x == (int)start.x)
			result.impactSurfaceNormal.x = -2;
		if (start.y == (int)start.y)
			result.impactSurfaceNormal.y = -2;
		if (start.z == (int)start.z)
			result.impactSurfaceNormal.z = -2;
		return result;
	}
	Vector3 rayDisplacement = end - start;
	float tDeltaX = abs(1.0f / rayDisplacement.x);
	int tileStepX = rayDisplacement.x > 0 ? 1 : -1;
	int offsetToLeadingEdgeX = (tileStepX + 1) / 2;
	float firstVerticalIntersectionX = (float)(rayPosition.x + offsetToLeadingEdgeX);
	float tOfNextXCrossing = abs(firstVerticalIntersectionX - start.x) * tDeltaX;

	float tDeltaY = abs(1.0f / rayDisplacement.y);
	int tileStepY = rayDisplacement.y > 0 ? 1 : -1;
	int offsetToLeadingEdgeY = (tileStepY + 1) / 2;
	float firstVerticalIntersectionY = (float)(rayPosition.y + offsetToLeadingEdgeY);
	float tOfNextYCrossing = abs(firstVerticalIntersectionY - start.y) * tDeltaY;

	float tDeltaZ = abs(1.0f / rayDisplacement.z);
	int tileStepZ = rayDisplacement.z > 0 ? 1 : -1;
	int offsetToLeadingEdgeZ = (tileStepZ + 1) / 2;
	float firstVerticalIntersectionZ = (float)(rayPosition.z + offsetToLeadingEdgeZ);
	float tOfNextZCrossing = abs(firstVerticalIntersectionZ - start.z) * tDeltaZ;

	while (true)
	{
		if ((tOfNextXCrossing < tOfNextYCrossing) && tOfNextXCrossing < tOfNextZCrossing)
		{
			if (tOfNextXCrossing > 1)
			{
				result.didImpact = false;
				result.impactTileCoords = rayPosition;
				return result;
			}
			rayPosition.x += tileStepX;
			Block* block = GetBlockFromWorldCoords(rayPosition);
			if (!block)
			{
				result.didImpact = false;
				result.impactTileCoords = rayPosition;
				return result;
			}
			uchar currentBlockType = block->m_type;
			if (BlockDefinition::GetDefinition(currentBlockType)->IsSolid())
			{
				result.didImpact = true;
				result.impactPosition = start + (rayDisplacement * tOfNextXCrossing);
				result.impactFraction = tOfNextXCrossing;
				result.impactTileCoords = rayPosition;
				result.impactSurfaceNormal = tileStepX < 0 ? Vector3Int(1, 0, 0) : Vector3Int(-1, 0, 0);
				return result;
			}
			tOfNextXCrossing += tDeltaX;
		}
		else if ((tOfNextYCrossing < tOfNextXCrossing) && tOfNextYCrossing < tOfNextZCrossing)
		{
			if (tOfNextYCrossing > 1)
			{
				result.didImpact = false;
				result.impactTileCoords = rayPosition;
				return result;
			}
			rayPosition.y += tileStepY;
			Block* block = GetBlockFromWorldCoords(rayPosition);
			if (!block)
			{
				result.didImpact = false;
				result.impactTileCoords = rayPosition;
				return result;
			}
			uchar currentBlockType = block->m_type;
			if (BlockDefinition::GetDefinition(currentBlockType)->IsSolid())
			{
				result.didImpact = true;
				result.impactPosition = start + (rayDisplacement * tOfNextYCrossing);
				result.impactFraction = tOfNextYCrossing;
				result.impactTileCoords = rayPosition;
				result.impactSurfaceNormal = tileStepY < 0 ? Vector3Int(0, 1, 0) : Vector3Int(0, -1, 0);
				return result;
			}
			tOfNextYCrossing += tDeltaY;
		}
		else
		{
			if (tOfNextZCrossing > 1)
			{
				result.didImpact = false;
				result.impactTileCoords = rayPosition;
				return result;
			}
			rayPosition.z += tileStepZ;
			Block* block = GetBlockFromWorldCoords(rayPosition);
			if (!block)
			{
				result.didImpact = false;
				result.impactTileCoords = rayPosition;
				return result;
			}
			uchar currentBlockType = block->m_type;
			if (BlockDefinition::GetDefinition(currentBlockType)->IsSolid())
			{
				result.didImpact = true;
				result.impactPosition = start + (rayDisplacement * tOfNextZCrossing);
				result.impactFraction = tOfNextZCrossing;
				result.impactTileCoords = rayPosition;
				result.impactSurfaceNormal = tileStepZ < 0 ? Vector3Int(0, 0, 1) : Vector3Int(0, 0, -1);
				return result;
			}
			tOfNextZCrossing += tDeltaZ;
		}
	}
}

//-----------------------------------------------------------------------------------
int World::GetNumActiveChunks()
{
	return m_activeChunks.size();
}

//-----------------------------------------------------------------------------------
void World::ParseChunksInSquare(const AABB2 bounds)
{
	ChunkCoords mins = ChunkCoords(static_cast<int>(bounds.mins.x), static_cast<int>(bounds.mins.y));
	ChunkCoords maxs = ChunkCoords(static_cast<int>(bounds.maxs.x), static_cast<int>(bounds.maxs.y));
	ChunkCoords topLeft = ChunkCoords(mins.x, maxs.y);
	ChunkCoords bottomRight = ChunkCoords(maxs.x, mins.y);
	m_chunkRenderingOffsets.push_back(mins);
	m_chunkRenderingOffsets.push_back(maxs);
	m_chunkRenderingOffsets.push_back(topLeft);
	m_chunkRenderingOffsets.push_back(bottomRight);

	for (int i = 1; i < maxs.x; i++)
	{
		m_chunkRenderingOffsets.push_back(mins + (ChunkCoords::UNIT_X * i));
		m_chunkRenderingOffsets.push_back(maxs - (ChunkCoords::UNIT_X * i));
		m_chunkRenderingOffsets.push_back(topLeft + (ChunkCoords::UNIT_X * i));
		m_chunkRenderingOffsets.push_back(bottomRight - (ChunkCoords::UNIT_X * i));
		m_chunkRenderingOffsets.push_back(mins + (ChunkCoords::UNIT_Y * i));
		m_chunkRenderingOffsets.push_back(maxs - (ChunkCoords::UNIT_Y * i));
		m_chunkRenderingOffsets.push_back(topLeft - (ChunkCoords::UNIT_Y * i));
		m_chunkRenderingOffsets.push_back(bottomRight + (ChunkCoords::UNIT_Y * i));
	}
	m_chunkRenderingOffsets.push_back(mins + (ChunkCoords::UNIT_X * maxs.x));
	m_chunkRenderingOffsets.push_back(mins + (ChunkCoords::UNIT_Y * maxs.y));
	m_chunkRenderingOffsets.push_back(maxs + (ChunkCoords::UNIT_X * mins.x));
	m_chunkRenderingOffsets.push_back(maxs + (ChunkCoords::UNIT_Y * mins.y));

	AABB2 newBounds = AABB2(bounds.mins + Vector2::ONE, bounds.maxs - Vector2::ONE);
	if (bounds.mins == Vector2::ZERO)
	{
		return;
	}
	else
	{
		ParseChunksInSquare(newBounds);
	}
}

//-----------------------------------------------------------------------------------
void World::CreateRenderingOffsetList(Player* player)
{
	ChunkCoords playerChunk = GetPlayerChunkCoords(player);
	AABB2 squareRadius = AABB2(Vector2((float)-ACTIVE_RADIUS, (float)-ACTIVE_RADIUS), Vector2((float)ACTIVE_RADIUS, (float)ACTIVE_RADIUS));
	ParseChunksInSquare(squareRadius);
}

//-----------------------------------------------------------------------------------
void World::AddToSaveQueue(Chunk* flushedChunk)
{
	//Before we add it to the save queue, we need to pull the chunk out of any processing stages it's in.
	for (auto iter = m_dirtyChunks.begin(); iter != m_dirtyChunks.end();)
	{
		if (iter->chunk == flushedChunk)
		{
			iter = m_dirtyChunks.erase(iter);
		}
		else
		{
			iter++;
		}
	}
	//Remove any blocks that were marked as dirty lighting by the chunk's creation.
	for (auto iter = m_dirtyBlocks.begin(); iter != m_dirtyBlocks.end();)
	{
		if (iter->m_chunk == flushedChunk)
		{
			iter = m_dirtyBlocks.erase(iter);
		}
		else
		{
			iter++;
		}
	}
	EnterCriticalSection(&g_diskIOCriticalSection);
	{
		g_requestedChunkSaveDeque.push_back(flushedChunk);
	}
	LeaveCriticalSection(&g_diskIOCriticalSection);
	m_pendingRequests.erase(flushedChunk->m_chunkPosition);
}

//-----------------------------------------------------------------------------------
void World::PickUpCompletedChunks()
{
	Chunk* newChunk = nullptr;
	EnterCriticalSection(&g_chunkListsCriticalSection);
	{
		if (g_readyToActivateChunksDeque.size() > 0)
		{
			newChunk = g_readyToActivateChunksDeque.front();
			if (newChunk->m_world == this)
			{
				g_readyToActivateChunksDeque.pop_front();
			}
			else
			{
				newChunk = nullptr;
			}
		}
	}
	LeaveCriticalSection(&g_chunkListsCriticalSection);
	if (newChunk)
	{
		ChunkCoords chunkPosition = newChunk->m_chunkPosition;
		m_activeChunks[chunkPosition] = newChunk;
		newChunk->DirtyAndAddToDirtyList();
		newChunk->CalculateSkyLighting();
		HookUpChunkPointers(m_activeChunks[chunkPosition]);
		m_chunkAddRemoveBalance++;
		DebuggerPrintf("[%i] World [%i]: Claiming Chunk %i,%i\n", g_frameNumber, m_worldID, chunkPosition.x, chunkPosition.y);
	}
}

//--------------------------------------------------------------------------------
void World::UpdateLighting()
{
	DebuggerPrintf("[%i] World [%i]: Updating %i blocks for lighting intially.\n", g_frameNumber, m_worldID, m_dirtyBlocks.size());
	unsigned int numberOfDirtyBlocks = 0;
	while (!m_dirtyBlocks.empty())
	{
		BlockInfo bi = m_dirtyBlocks.front();
		m_dirtyBlocks.pop_front();
		bi.GetBlock()->SetDirty(false);
		UpdateLightingForBlock(bi);
		numberOfDirtyBlocks++;
	}
	DebuggerPrintf("[%i] World [%i]: Actually updated %i blocks during lighting.\n", g_frameNumber, m_worldID, numberOfDirtyBlocks);
}

//--------------------------------------------------------------------------------
void World::UpdateLightingForBlock(const BlockInfo& bi)
{
	RGBA ideal = GetIdealLightForBlock(bi);
	RGBA current = bi.GetBlock()->GetRGBALightValue();
	if (ideal == current)
		return;
	Block* block = bi.GetBlock();
	if (block)
	{
		block->SetLightValue(ideal);
	}
	SetBlockNeighborsDirty(bi);
	bi.m_chunk->DirtyAndAddToDirtyList();
}

//-----------------------------------------------------------------------------------
void World::MarkAsLightingDirty(BlockInfo& bi)
{
	Block* block = bi.GetBlock();
	if (!block)
		return;
	if (block->IsDirty())
		return;
	BlockInfo::SetDirtyFlagAndAddToDirtyList(bi);

}

//-----------------------------------------------------------------------------------
void World::SetBlockNeighborsDirty(const BlockInfo& info)
{
	if (info.m_index == BlockInfo::INVALID_INDEX)
	{
		return;
	}
	BlockInfo eastBlock = info.GetEast();
	if (eastBlock.m_index != BlockInfo::INVALID_INDEX && !BlockDefinition::GetDefinition(eastBlock.GetBlock()->m_type)->m_isOpaque)
	{
		BlockInfo::SetDirtyFlagAndAddToDirtyList(eastBlock);
	}
	BlockInfo westBlock = info.GetWest();
	if (westBlock.m_index != BlockInfo::INVALID_INDEX && !BlockDefinition::GetDefinition(westBlock.GetBlock()->m_type)->m_isOpaque)
	{
		BlockInfo::SetDirtyFlagAndAddToDirtyList(westBlock);
	}
	BlockInfo northBlock = info.GetNorth();
	if (northBlock.m_index != BlockInfo::INVALID_INDEX && !BlockDefinition::GetDefinition(northBlock.GetBlock()->m_type)->m_isOpaque)
	{
		BlockInfo::SetDirtyFlagAndAddToDirtyList(northBlock);
	}
	BlockInfo southBlock = info.GetSouth();
	if (southBlock.m_index != BlockInfo::INVALID_INDEX && !BlockDefinition::GetDefinition(southBlock.GetBlock()->m_type)->m_isOpaque)
	{
		BlockInfo::SetDirtyFlagAndAddToDirtyList(southBlock);
	}
	BlockInfo belowBlock = info.GetBelow();
	if (belowBlock.m_index != BlockInfo::INVALID_INDEX && !BlockDefinition::GetDefinition(belowBlock.GetBlock()->m_type)->m_isOpaque)
	{
		BlockInfo::SetDirtyFlagAndAddToDirtyList(belowBlock);
	}
	BlockInfo aboveBlock = info.GetAbove();
	if (aboveBlock.m_index != BlockInfo::INVALID_INDEX && !BlockDefinition::GetDefinition(aboveBlock.GetBlock()->m_type)->m_isOpaque)
	{
		BlockInfo::SetDirtyFlagAndAddToDirtyList(aboveBlock);
	}
	//If we're an edge block, we need to inform the other chunk that its VA's are out of date
	if (info.GetBlock()->IsEdgeBlock())
	{
		BlockInfo east = info.GetEast();
		BlockInfo west = info.GetWest();
		BlockInfo north = info.GetNorth();
		BlockInfo south = info.GetSouth();
		if (info.IsOnEast() && east.m_chunk)
		{
			east.m_chunk->DirtyAndAddToDirtyList();
		}
		else if (info.IsOnWest() && west.m_chunk)
		{
			west.m_chunk->DirtyAndAddToDirtyList();
		}

		if (info.IsOnNorth() && north.m_chunk)
		{
			north.m_chunk->DirtyAndAddToDirtyList();
		}
		else if (info.IsOnSouth() && south.m_chunk)
		{
			south.m_chunk->DirtyAndAddToDirtyList();
		}
	}
}

//-----------------------------------------------------------------------------------
RGBA World::GetIdealLightForBlock(const BlockInfo& bi)
{
	BlockDefinition* definition = BlockDefinition::GetDefinition(bi.GetBlock()->m_type);
	RGBA myLight = RGBA(definition->m_illumination);
	if (definition->m_isOpaque)
	{
		return myLight;
	}
	RGBA brightestNeighbor = GetBrightestNeighbor(bi);

	RGBA highestNeighborMinusOneStep = brightestNeighbor - 0x0F;
	uchar highestNeighborMinusOneStepRed = highestNeighborMinusOneStep.red > brightestNeighbor.red ? 0x00 : highestNeighborMinusOneStep.red;
	uchar highestNeighborMinusOneStepGreen = highestNeighborMinusOneStep.green > brightestNeighbor.green ? 0x00 : highestNeighborMinusOneStep.green;
	uchar highestNeighborMinusOneStepBlue = highestNeighborMinusOneStep.blue > brightestNeighbor.blue ? 0x00 : highestNeighborMinusOneStep.blue;
	uchar highestFilteredForOpacityRed = highestNeighborMinusOneStepRed - definition->m_opacity.red;
	uchar highestFilteredForOpacityGreen = highestNeighborMinusOneStepGreen - definition->m_opacity.green;
	uchar highestFilteredForOpacityBlue = highestNeighborMinusOneStepBlue - definition->m_opacity.blue;
	highestFilteredForOpacityRed = highestFilteredForOpacityRed > highestNeighborMinusOneStepRed ? 0x00 : highestFilteredForOpacityRed;
	highestFilteredForOpacityGreen = highestFilteredForOpacityGreen > highestNeighborMinusOneStepGreen ? 0x00 : highestFilteredForOpacityGreen;
	highestFilteredForOpacityBlue = highestFilteredForOpacityBlue > highestNeighborMinusOneStepBlue ? 0x00 : highestFilteredForOpacityBlue;

	RGBA skylight = bi.GetBlock()->IsSky() ? m_skyLight : RGBA::BLACK;
	uchar idealRed = myLight.red > skylight.red ? myLight.red : skylight.red;
	uchar idealGreen = myLight.green > skylight.green ? myLight.green : skylight.green;
	uchar idealBlue = myLight.blue > skylight.blue ? myLight.blue : skylight.blue;

	idealRed = highestFilteredForOpacityRed > idealRed ? highestFilteredForOpacityRed : idealRed;
	idealGreen = highestFilteredForOpacityGreen > idealGreen ? highestFilteredForOpacityGreen : idealGreen;
	idealBlue = highestFilteredForOpacityBlue > idealBlue ? highestFilteredForOpacityBlue : idealBlue;

	return RGBA::CreateFromUChars(idealRed, idealGreen, idealBlue, 0xFF);
}

//-----------------------------------------------------------------------------------
RGBA World::GetBrightestNeighbor(const BlockInfo& info)
{
	RGBA brightestLightValue = RGBA::BLACK;
	if (info.m_index == BlockInfo::INVALID_INDEX)
	{
		return brightestLightValue;
	}
	for (Direction neighborDirection : BlockInfo::directions)
	{
		BlockInfo neighborBlock = info.GetNeighbor(neighborDirection);
		if (neighborBlock.m_index != BlockInfo::INVALID_INDEX)
		{
			RGBA neighborLight = neighborBlock.GetBlock()->GetRGBALightValue();
			uchar brightestRedValue = neighborLight.red > brightestLightValue.red ? neighborLight.red : brightestLightValue.red;
			uchar brightestGreenValue = neighborLight.green > brightestLightValue.green ? neighborLight.green : brightestLightValue.green;
			uchar brightestBlueValue = neighborLight.blue > brightestLightValue.blue ? neighborLight.blue : brightestLightValue.blue;
			brightestLightValue = RGBA::CreateFromUChars(brightestRedValue, brightestGreenValue, brightestBlueValue, 0x00);
		}
	}
	return brightestLightValue;
}

//-----------------------------------------------------------------------------------
void ChunkGenerationThreadMain()
{
	while (!g_isQuitting)
	{
		Sleep(0);
		bool gotCoords = false;
		PrioritizedChunkCoords coords;
		EnterCriticalSection(&g_chunkListsCriticalSection);
		if (!g_requestedChunkGenerationSet.empty())
		{
			gotCoords = true;
			coords = (*g_requestedChunkGenerationSet.begin());
			g_requestedChunkGenerationSet.erase(coords);
		}
		LeaveCriticalSection(&g_chunkListsCriticalSection);
		if (!gotCoords)
			continue;
		Chunk* newChunk = new Chunk(coords.chunkCoords, coords.world);
		EnterCriticalSection(&g_chunkListsCriticalSection);
		{
			g_readyToActivateChunksDeque.emplace_back(newChunk);
		}
		LeaveCriticalSection(&g_chunkListsCriticalSection);
	}
	
}

//-----------------------------------------------------------------------------------
void ChunkIOThreadMain()
{
	while (!g_isQuitting)
	{
		Sleep(0);
		bool gotCoords = false;
		Chunk* chunkToSave = nullptr;
		PrioritizedChunkCoords chunkToLoadCoords;

		EnterCriticalSection(&g_diskIOCriticalSection);
		{
			if (!g_requestedChunkLoadSet.empty())
			{
				gotCoords = true;
				chunkToLoadCoords = *g_requestedChunkLoadSet.begin();
				g_requestedChunkLoadSet.erase(chunkToLoadCoords);
			}
			if (!g_requestedChunkSaveDeque.empty())
			{
				chunkToSave = g_requestedChunkSaveDeque.back();		
				if (chunkToSave)
				{
					g_requestedChunkSaveDeque.pop_back();
				}
			}
		}
		LeaveCriticalSection(&g_diskIOCriticalSection);

		if (gotCoords)
		{
			Chunk* loadedChunk = World::LoadChunk(chunkToLoadCoords.world->m_worldID,chunkToLoadCoords.chunkCoords);
			if (loadedChunk)
			{
				EnterCriticalSection(&g_chunkListsCriticalSection);
				{
					g_readyToActivateChunksDeque.emplace_back(loadedChunk);
				}
				LeaveCriticalSection(&g_chunkListsCriticalSection);
			}
		}
		if (chunkToSave)
		{
			World::SaveChunk(chunkToSave);
			delete(chunkToSave);
		}
	}
}
