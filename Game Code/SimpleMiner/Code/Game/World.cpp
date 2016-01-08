#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <thread>

#include "Game/World.hpp"
#include "Game/BlockDefinition.h"
#include "Game/TheGame.hpp"
#include "Game/Camera3D.hpp"
#include "Game/Player.hpp"
#include "Engine/Input/InputOutputUtils.hpp"
#include "Engine/Renderer/Face.hpp"
#include "Engine/Renderer/Vertex.hpp"
#include <algorithm>
#include <regex>

std::vector<Vertex_PCT> g_debugPoints;
std::set<ChunkCoords> g_requestedChunkGenerationList;
std::set<ChunkCoords> g_requestedChunkLoadList;
std::set<ChunkCoords> g_chunksOnDisk;
std::deque<Chunk*> g_requestedChunkSaveList;
std::deque<Chunk*> g_readyToActivateChunks; 
ProfilingID g_generationProfiling;
ProfilingID g_loadingProfiling;
ProfilingID g_savingProfiling;
ProfilingID g_vaBuildingProfiling;
ProfilingID g_temporaryProfiling;
extern CRITICAL_SECTION g_chunkListsCriticalSection;
extern CRITICAL_SECTION g_diskIOCriticalSection;

World::World()
: m_chunkAddRemoveBalance(0)
, m_isChunkGenerationThreadDone(false)
, m_chunkGenerationThread(ChunkGenerationThreadMain)
, m_diskIOThread(ChunkIOThreadMain)
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

void World::UpdateVertexArrays()
{
	for (auto currentChunkPair : m_activeChunks)
	{
		Chunk* currentChunk = currentChunkPair.second;
		currentChunk->UpdateVAIfDirty();
	}
}

void World::RequestNeededChunks()
{
	ChunkCoords chunkToGenerate(0, 0);
	bool shouldGenerateChunk = GetHighestPriorityMissingChunk(chunkToGenerate);
	if (shouldGenerateChunk)
	{
		RequestChunk(chunkToGenerate);
	}
}

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

void World::FindAllChunksOnDisk()
{
	std::vector<std::string>* fileNames = GetFileNamesInFolder("Data\\SaveData\\Save0\\*");
	std::regex chunkFile ("-?[0-9]+,-?[0-9]+.*\\.chunk");
	for (std::string fileName : *fileNames)
	{
		if (regex_match(fileName, chunkFile))
		{
			ChunkCoords fileCoords;
			char *fileCStr = new char[fileName.length() + 1];
			strcpy(fileCStr, fileName.c_str());
			char* firstNumber = strtok(fileCStr, ",.");
			fileCoords.x = atoi(firstNumber);
			char* secondNumber = strtok(NULL, ",.");
			fileCoords.y = atoi(secondNumber);
			g_chunksOnDisk.insert(fileCoords);
			delete [] fileCStr;
		}
	}
	delete fileNames;
}

bool World::IsChunkOnDisk(ChunkCoords & chunkToGenerate)
{
	return (g_chunksOnDisk.find(chunkToGenerate) != g_chunksOnDisk.end());
}

void World::RequestChunk(ChunkCoords &chunkToGenerate)
{
	bool isOnDisk = IsChunkOnDisk(chunkToGenerate);
	if (isOnDisk)
	{
		EnterCriticalSection(&g_diskIOCriticalSection);
		{
			g_requestedChunkLoadList.insert(chunkToGenerate);
		}
		LeaveCriticalSection(&g_diskIOCriticalSection);
	}
	else
	{
		EnterCriticalSection(&g_chunkListsCriticalSection);
		{
			g_requestedChunkGenerationList.insert(chunkToGenerate);
		}
		LeaveCriticalSection(&g_chunkListsCriticalSection);
	}
	
}

Chunk* World::LoadChunk(ChunkCoords &chunkToGenerate)
{
	StartTiming(g_loadingProfiling);
	std::vector<uchar> chunkData;
	Chunk* loadedChunk = nullptr;

	bool fileLoaded = LoadBufferFromBinaryFile(chunkData, Stringf("Data\\SaveData\\Save0\\%i,%i.chunk", chunkToGenerate.x, chunkToGenerate.y));
	if (fileLoaded)
	{
		loadedChunk = new Chunk(chunkToGenerate, chunkData, TheGame::instance->m_world);
	}
	EndTiming(g_loadingProfiling);
	return loadedChunk;
}

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
			if (currentChunk->IsInFrustum(TheGame::instance->m_playerCamera->GetForwardXYZ(), TheGame::instance->m_player->m_position))
			{
				currentChunk->Render();
			}
		}
	}
}

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
	currentChunk->m_isDirty = true;
	BlockInfo::SetDirtyFlagAndAddToDirtyList(info);
	if (block->IsEdgeBlock())
	{
		BlockInfo east = info.GetEast();
		BlockInfo west = info.GetWest();
		BlockInfo north = info.GetNorth();
		BlockInfo south = info.GetSouth();
		if (info.IsOnEast() && east.m_chunk)
		{
			east.m_chunk->m_isDirty = true;
		}
		else if (info.IsOnWest() && west.m_chunk)
		{
			west.m_chunk->m_isDirty = true;
		}

		if (info.IsOnNorth() && north.m_chunk)
		{
			north.m_chunk->m_isDirty = true;
		}
		else if (info.IsOnSouth() && south.m_chunk)
		{
			south.m_chunk->m_isDirty = true;
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
	if (info.GetAbove().GetBlock()->m_type == BlockType::WATER
		|| info.GetWest().GetBlock()->m_type == BlockType::WATER
		|| info.GetEast().GetBlock()->m_type == BlockType::WATER
		|| info.GetNorth().GetBlock()->m_type == BlockType::WATER
		|| info.GetSouth().GetBlock()->m_type == BlockType::WATER)
	{
		block->m_type = BlockType::WATER;
	}
	else
	{
		block->m_type = BlockType::AIR;
	}
	info.m_chunk->m_isDirty = true;
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
			east.m_chunk->m_isDirty = true;
		}
		else if (info.IsOnWest() && west.m_chunk)
		{
			west.m_chunk->m_isDirty = true;
		}

		if (info.IsOnNorth() && north.m_chunk)
		{
			north.m_chunk->m_isDirty = true;
		}
		else if (info.IsOnSouth() && south.m_chunk)
		{
			south.m_chunk->m_isDirty = true;
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

bool World::GetHighestPriorityMissingChunk(ChunkCoords& out_chunkCoords)
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
			WorldPosition worldPosCandidateChunk = GetWorldPositionFromChunkCoords(candidateChunkCoords);
			WorldPosition adjustedPlayerPosition = TheGame::instance->m_playerCamera->m_position;
			adjustedPlayerPosition.z = 0.0f;
			float distToChunk = MathUtils::CalcDistSquaredBetweenPoints(adjustedPlayerPosition, worldPosCandidateChunk);
			if (distToChunk < radiusSquared && m_activeChunks.find(candidateChunkCoords) == m_activeChunks.end())
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
	out_chunkCoords = mostUrgent;
	return shouldGenerate;
}

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

void World::SaveChunk(Chunk* chunkToUnload)
{
	StartTiming(g_savingProfiling);
	std::vector<uchar> chunkData;
	chunkToUnload->GenerateSaveData(chunkData);
	SaveBufferToBinaryFile(chunkData, Stringf("Data\\SaveData\\Save0\\%i,%i.chunk", chunkToUnload->m_chunkPosition.x, chunkToUnload->m_chunkPosition.y));
	g_chunksOnDisk.insert(chunkToUnload->m_chunkPosition);
	EndTiming(g_savingProfiling);
}

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
		Chunk* neighborChunk = eastChunk->second;
		chunkToHookUp->m_eastChunk = neighborChunk;
		neighborChunk->m_westChunk = chunkToHookUp;
		neighborChunk->FlagEdgesAsDirtyLighting();
		neighborChunk->m_isDirty = true;
	}
	auto westChunk = m_activeChunks.find(westChunkPos);
	if (westChunk != m_activeChunks.end())
	{
		chunkToHookUp->m_westChunk = westChunk->second;
		Chunk* chunk = westChunk->second;
		chunk->m_eastChunk = chunkToHookUp;
		chunk->FlagEdgesAsDirtyLighting();
		chunk->m_isDirty = true;
	}
	auto northChunk = m_activeChunks.find(northChunkPos);
	if (northChunk != m_activeChunks.end())
	{
		chunkToHookUp->m_northChunk = northChunk->second;
		Chunk* chunk = northChunk->second;
		chunk->m_southChunk = chunkToHookUp;
		chunk->FlagEdgesAsDirtyLighting();
		chunk->m_isDirty = true;
	}
	auto southChunk = m_activeChunks.find(southChunkPos);
	if (southChunk != m_activeChunks.end())
	{
		chunkToHookUp->m_southChunk = southChunk->second;
		Chunk* chunk = southChunk->second;
		chunk->m_northChunk = chunkToHookUp;
		chunk->FlagEdgesAsDirtyLighting();
		chunk->m_isDirty = true;
	}
}

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
		Chunk* chunk = eastChunk->second;
		chunk->m_westChunk = nullptr;
	}
	auto westChunk = m_activeChunks.find(westChunkPos);
	if (westChunk != m_activeChunks.end())
	{
		Chunk* chunk = westChunk->second;
		chunk->m_eastChunk = nullptr;
	}
	auto northChunk = m_activeChunks.find(northChunkPos);
	if (northChunk != m_activeChunks.end())
	{
		Chunk* chunk = northChunk->second;
		chunk->m_southChunk = nullptr;
	}
	auto southChunk = m_activeChunks.find(southChunkPos);
	if (southChunk != m_activeChunks.end())
	{
		Chunk* chunk = southChunk->second;
		chunk->m_northChunk = nullptr;
	}
}

ChunkCoords World::GetPlayerChunkCoords() const
{
	WorldPosition playerPosition = TheGame::instance->m_player->m_position;
	return ChunkCoords(static_cast<int>(floor(playerPosition.x / Chunk::BLOCKS_WIDE_X)), static_cast<int>(floor(playerPosition.y / Chunk::BLOCKS_WIDE_X)));
}

ChunkCoords World::GetPlayerChunkCoords(Player* player) const
{
	WorldPosition playerPosition = player->m_position;
	return ChunkCoords(static_cast<int>(floor(playerPosition.x / Chunk::BLOCKS_WIDE_X)), static_cast<int>(floor(playerPosition.y / Chunk::BLOCKS_WIDE_X)));
}

WorldPosition World::GetWorldPositionFromChunkCoords(const ChunkCoords& chunkCoords) const
{
	const float halfX = Chunk::BLOCKS_WIDE_X / 2.0f;
	const float halfY = Chunk::BLOCKS_WIDE_Y / 2.0f;
	float worldX = (chunkCoords.x * Chunk::BLOCKS_WIDE_X) + halfX;
	float worldY = (chunkCoords.y * Chunk::BLOCKS_WIDE_Y) + halfY;
	return WorldPosition(worldX, worldY, 0.0f);
}

Block* World::GetBlockFromWorldPosition(const WorldPosition& worldPos) const
{
	return GetBlockFromWorldCoords(WorldCoords(static_cast<int>(floor(worldPos.x)), static_cast<int>(floor(worldPos.y)), static_cast<int>(floor(worldPos.z))));
}

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

RaycastResult3D World::Raycast(const Vector3& start, const Vector3& end) const
{
	RaycastResult3D result;
	result.didImpact = false;
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
		result.impactFraction = 0.0f;
		result.impactPosition = start;
		result.impactTileCoords = rayPosition;
		result.impactSurfaceNormal = Vector3Int(0, 0, 0);
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
			uchar blockType = block->m_type;
			if (BlockDefinition::GetDefinition(blockType)->IsSolid())
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
			uchar blockType = block->m_type;
			if (BlockDefinition::GetDefinition(blockType)->IsSolid())
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
			uchar blockType = block->m_type;
			if (BlockDefinition::GetDefinition(blockType)->IsSolid())
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

int World::GetNumActiveChunks()
{
	return m_activeChunks.size();
}

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

void World::CreateRenderingOffsetList(Player* player)
{
	ChunkCoords playerChunk = GetPlayerChunkCoords(player);
	AABB2 squareRadius = AABB2(Vector2((float)-ACTIVE_RADIUS, (float)-ACTIVE_RADIUS), Vector2((float)ACTIVE_RADIUS, (float)ACTIVE_RADIUS));
	ParseChunksInSquare(squareRadius);
}

void World::AddToSaveQueue(Chunk* flushedChunk)
{
	EnterCriticalSection(&g_diskIOCriticalSection);
	{
		g_requestedChunkSaveList.push_back(flushedChunk);
	}
	LeaveCriticalSection(&g_diskIOCriticalSection);
}

void World::PickUpCompletedChunks()
{
	Chunk* newChunk = nullptr;
	EnterCriticalSection(&g_chunkListsCriticalSection);
	{
		if (g_readyToActivateChunks.size() > 0)
		{
			newChunk = g_readyToActivateChunks.front();
			g_readyToActivateChunks.pop_front();
		}
	}
	LeaveCriticalSection(&g_chunkListsCriticalSection);
	if (newChunk)
	{
		ChunkCoords chunkPosition = newChunk->m_chunkPosition;
		m_activeChunks[chunkPosition] = newChunk;
		newChunk->SetEdgeBits();
		newChunk->CalculateSkyLighting();
		HookUpChunkPointers(m_activeChunks[chunkPosition]);
		m_chunkAddRemoveBalance++;
	}
}

void World::UpdateLighting()
{
	while (!m_dirtyBlocks.empty())
	{
		BlockInfo bi = m_dirtyBlocks.front();
		m_dirtyBlocks.pop_front();
		bi.GetBlock()->SetDirty(false);
		UpdateLightingForBlock(bi);
	}
}

void World::UpdateLightingForBlock(const BlockInfo& bi)
{
	RGBA ideal = RGBA();
	ideal.red = GetIdealRedLightForBlock(bi);
	ideal.green = GetIdealGreenLightForBlock(bi);
	ideal.blue = GetIdealBlueLightForBlock(bi);
	RGBA current = RGBA();
	current.red = bi.GetBlock()->GetRedLightValue();
	current.green = bi.GetBlock()->GetGreenLightValue();
	current.blue = bi.GetBlock()->GetBlueLightValue();
	if (ideal == current)
		return;
	Block* block = bi.GetBlock();
	if (block)
	{
		block->SetRedLightValue(ideal.red);
		block->SetGreenLightValue(ideal.green);
		block->SetBlueLightValue(ideal.blue);
	}
	SetBlockNeighborsDirty(bi);
	bi.m_chunk->m_isDirty = true;
}

void World::MarkAsLightingDirty(BlockInfo& bi)
{
	Block* block = bi.GetBlock();
	if (!block)
		return;
	if (block->IsDirty())
		return;
	BlockInfo::SetDirtyFlagAndAddToDirtyList(bi);

}

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
			east.m_chunk->m_isDirty = true;
		}
		else if (info.IsOnWest() && west.m_chunk)
		{
			west.m_chunk->m_isDirty = true;
		}

		if (info.IsOnNorth() && north.m_chunk)
		{
			north.m_chunk->m_isDirty = true;
		}
		else if (info.IsOnSouth() && south.m_chunk)
		{
			south.m_chunk->m_isDirty = true;
		}
	}
}

uchar World::GetIdealRedLightForBlock(const BlockInfo& bi)
{
	BlockDefinition* definition = BlockDefinition::GetDefinition(bi.GetBlock()->m_type);
	if (definition->m_isOpaque)
	{
		return RGBA::GetRed(definition->m_illumination);
	}
	uchar myLight = RGBA::GetRed(definition->m_illumination);
	uchar brightestNeighbor = GetBrightestRedNeighbor(bi);
	uchar highestNeighborMinusOneStep = brightestNeighbor - 0x0F;
	highestNeighborMinusOneStep = highestNeighborMinusOneStep > brightestNeighbor ? 0x00 : highestNeighborMinusOneStep;
	uchar highestFilteredForOpacity = highestNeighborMinusOneStep - definition->m_opacity.red;
	highestFilteredForOpacity = highestFilteredForOpacity > highestNeighborMinusOneStep ? 0x00 : highestFilteredForOpacity;
	uchar skylight = bi.GetBlock()->IsSky() ? RGBA::GetRed(SKY_LIGHT) : 0x00;
	uchar ideal = myLight > skylight ? myLight : skylight;
	return highestFilteredForOpacity > ideal ? highestFilteredForOpacity : ideal;
}

uchar World::GetIdealGreenLightForBlock(const BlockInfo& bi)
{
	BlockDefinition* definition = BlockDefinition::GetDefinition(bi.GetBlock()->m_type);
	if (definition->m_isOpaque)
	{
		return RGBA::GetGreen(definition->m_illumination);
	}
	uchar myLight = RGBA::GetGreen(definition->m_illumination);
	uchar brightestNeighbor = GetBrightestGreenNeighbor(bi);
	uchar highestNeighborMinusOneStep = brightestNeighbor - 0x0F;
	highestNeighborMinusOneStep = highestNeighborMinusOneStep > brightestNeighbor ? 0x00 : highestNeighborMinusOneStep;
	uchar highestFilteredForOpacity = highestNeighborMinusOneStep - definition->m_opacity.green;
	highestFilteredForOpacity = highestFilteredForOpacity > highestNeighborMinusOneStep ? 0x00 : highestFilteredForOpacity;
	uchar skylight = bi.GetBlock()->IsSky() ? RGBA::GetGreen(SKY_LIGHT) : 0x00;
	uchar ideal = myLight > skylight ? myLight : skylight;
	return highestFilteredForOpacity > ideal ? highestFilteredForOpacity : ideal;
}

uchar World::GetIdealBlueLightForBlock(const BlockInfo& bi)
{
	BlockDefinition* definition = BlockDefinition::GetDefinition(bi.GetBlock()->m_type);
	if (definition->m_isOpaque)
	{
		return RGBA::GetBlue(definition->m_illumination);
	}
	uchar myLight = RGBA::GetBlue(definition->m_illumination);
	uchar brightestNeighbor = GetBrightestBlueNeighbor(bi);
	uchar highestNeighborMinusOneStep = brightestNeighbor - 0x0F;
	highestNeighborMinusOneStep = highestNeighborMinusOneStep > brightestNeighbor ? 0x00 : highestNeighborMinusOneStep;
	uchar highestFilteredForOpacity = highestNeighborMinusOneStep - definition->m_opacity.blue;
	highestFilteredForOpacity = highestFilteredForOpacity > highestNeighborMinusOneStep ? 0x00 : highestFilteredForOpacity;
	uchar skylight = bi.GetBlock()->IsSky() ? RGBA::GetBlue(SKY_LIGHT) : 0x00;
	uchar ideal = myLight > skylight ? myLight : skylight;
	return highestFilteredForOpacity > ideal ? highestFilteredForOpacity : ideal;
}

uchar World::GetBrightestRedNeighbor(const BlockInfo& info)
{
	uchar lightValue = 0x00;
	if (info.m_index == BlockInfo::INVALID_INDEX)
	{
		return lightValue;
	}
	for (Direction neighborDirection : BlockInfo::directions)
	{
		BlockInfo neighborBlock = info.GetNeighbor(neighborDirection);
		if (neighborBlock.m_index != BlockInfo::INVALID_INDEX)
		{
			uchar current = neighborBlock.GetBlock()->GetRedLightValue();
			lightValue = current > lightValue ? current : lightValue;
		}
	}
	return lightValue;
}

uchar World::GetBrightestGreenNeighbor(const BlockInfo& info)
{
	uchar lightValue = 0x00;
	if (info.m_index == BlockInfo::INVALID_INDEX)
	{
		return lightValue;
	}
	for (Direction neighborDirection : BlockInfo::directions)
	{
		BlockInfo neighborBlock = info.GetNeighbor(neighborDirection);
		if (neighborBlock.m_index != BlockInfo::INVALID_INDEX)
		{
			uchar current = neighborBlock.GetBlock()->GetGreenLightValue();
			lightValue = current > lightValue ? current : lightValue;
		}
	}
	return lightValue;
}

uchar World::GetBrightestBlueNeighbor(const BlockInfo& info)
{
	uchar lightValue = 0x00;
	if (info.m_index == BlockInfo::INVALID_INDEX)
	{
		return lightValue;
	}
	for (Direction neighborDirection : BlockInfo::directions)
	{
		BlockInfo neighborBlock = info.GetNeighbor(neighborDirection);
		if (neighborBlock.m_index != BlockInfo::INVALID_INDEX)
		{
			uchar current = neighborBlock.GetBlock()->GetBlueLightValue();
			lightValue = current > lightValue ? current : lightValue;
		}
	}
	return lightValue;
}

void ChunkGenerationThreadMain()
{
	while (!g_isQuitting)
	{
		Sleep(0);
		bool gotCoords = false;
		ChunkCoords coords;
		EnterCriticalSection(&g_chunkListsCriticalSection);
		if (!g_requestedChunkGenerationList.empty())
		{
			gotCoords = true;
			coords = *g_requestedChunkGenerationList.begin();
		}
		LeaveCriticalSection(&g_chunkListsCriticalSection);
		if (!gotCoords)
			continue;
		Chunk* newChunk = new Chunk(coords, TheGame::instance->m_world);
		EnterCriticalSection(&g_chunkListsCriticalSection);
		{
			g_requestedChunkGenerationList.erase(coords);
			g_readyToActivateChunks.emplace_back(newChunk);
		}
		LeaveCriticalSection(&g_chunkListsCriticalSection);
	}
	
}

void ChunkIOThreadMain()
{
	while (!g_isQuitting)
	{
		Sleep(0);
		bool gotCoords = false;
		Chunk* chunkToSave = nullptr;
		ChunkCoords chunkToLoadCoords = ChunkCoords();

		EnterCriticalSection(&g_diskIOCriticalSection);
		{
			if (!g_requestedChunkLoadList.empty())
			{
				gotCoords = true;
				chunkToLoadCoords = *g_requestedChunkLoadList.begin();
			}
			if (!g_requestedChunkSaveList.empty())
			{
				chunkToSave = g_requestedChunkSaveList.back();		
				if (chunkToSave)
				{
					g_requestedChunkSaveList.pop_back();
				}
			}
		}
		LeaveCriticalSection(&g_diskIOCriticalSection);

		if (gotCoords)
		{
			Chunk* loadedChunk = World::LoadChunk(chunkToLoadCoords);
			if (loadedChunk)
			{
				EnterCriticalSection(&g_chunkListsCriticalSection);
				{
					g_readyToActivateChunks.emplace_back(loadedChunk);
				}
				LeaveCriticalSection(&g_chunkListsCriticalSection);
				
				EnterCriticalSection(&g_diskIOCriticalSection);
				{
					g_requestedChunkLoadList.erase(chunkToLoadCoords);
				}
				LeaveCriticalSection(&g_diskIOCriticalSection);
			}
		}
		if (chunkToSave)
		{
			World::SaveChunk(chunkToSave);
			delete(chunkToSave);
		}
	}
}
