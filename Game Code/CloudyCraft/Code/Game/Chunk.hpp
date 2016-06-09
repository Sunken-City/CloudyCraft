#pragma once

#include "Engine/Renderer/MeshRenderer.hpp"
#include "Game/Block.hpp"
#include "GameCommon.hpp"
#include <vector>
class Vector2Int;
class BlockInfo;
struct Vertex_PCT;

class Chunk
{
public:
	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
	Chunk(const ChunkCoords& chunkCoords, World* world);
	Chunk(const ChunkCoords& chunkCoords, std::vector<unsigned char>& data, World* world);
	~Chunk();

	//FUNCTIONS//////////////////////////////////////////////////////////////////////////
	void Update(float deltaTime);
	void Render() const;
	void GenerateChunk();
	bool IsInFrustum(const Vector3& cameraXYZ, const WorldPosition& playerPosition) const;
	void GenerateSaveData(std::vector<unsigned char>& data);
	void LoadChunkFromData(std::vector<unsigned char>& data);
	void AttemptCleanUpRenderData();

	//LIGHTING//////////////////////////////////////////////////////////////////////////
	void SetBlockDirtyAndAddToDirtyList(LocalIndex blockToDirtyIndex);
	void CalculateSkyLighting();
	void FlagEdgesAsDirtyLighting(Direction dir);

	//FACE VISIBILITY//////////////////////////////////////////////////////////////////////////
	void SetEdgeBits();
	void DirtyAndAddToDirtyList();
	void SetHighPriorityChunkDirtyAndAddToDirtyList();
	void GenerateVertexArray();

	//ACCESSORS AND CONVERSIONS//////////////////////////////////////////////////////////////////////////
	inline Block* GetBlock(LocalIndex index);
	inline Block* GetAbove(LocalIndex index);
	inline Block* GetBelow(LocalIndex index);
	inline Block* GetNorth(LocalIndex index);
	inline Block* GetSouth(LocalIndex index);
	inline Block* GetEast(LocalIndex index);
	inline Block* GetWest(LocalIndex index);
	inline LocalCoords GetLocalCoordsFromBlockIndex(LocalIndex index) const;
	inline WorldPosition GetWorldMinsForBlockIndex(LocalIndex index) const;
	inline LocalIndex GetBlockIndexFromLocalCoords(const LocalCoords& coords) const;
	inline BlockInfo GetBlockInfoFromLocalCoords(const LocalCoords& coords);
	LocalCoords GetLocalCoordsFromWorldCoords(const WorldCoords &coords) const;
	LocalCoords GetLocalCoordsFromWorldPosition(const WorldPosition &pos) const;
	WorldCoords GetWorldCoordsForBlockIndex(LocalIndex index) const;
	LocalIndex GetBlockIndexFromWorldCoords(const WorldCoords& pos) const;
	LocalIndex GetBlockIndexFromWorldPosition(const WorldPosition& pos) const;

	//CONSTANTS////////////////////////////////////////////////////////////////////////
	static const int CHUNK_BITS_X = 4;
	static const int CHUNK_BITS_Y = 4;
	static const int CHUNK_BITS_XY = CHUNK_BITS_X + CHUNK_BITS_Y;
	static const int CHUNK_BITS_Z = 7;
	static const int BLOCKS_WIDE_X = BIT(CHUNK_BITS_X);
	static const int BLOCKS_WIDE_Y = BIT(CHUNK_BITS_Y);
	static const int BLOCKS_TALL_Z = BIT(CHUNK_BITS_Z);
	static const int BLOCKS_PER_LAYER = BLOCKS_WIDE_X * BLOCKS_WIDE_Y;
	static const int BLOCKS_PER_CHUNK = BLOCKS_PER_LAYER * BLOCKS_TALL_Z;
	static const int LOCAL_X_MASK = BLOCKS_WIDE_X - 1;
	static const int LOCAL_Y_MASK = (BLOCKS_WIDE_Y - 1) << CHUNK_BITS_X;
	static const int LOCAL_Z_MASK = (BLOCKS_TALL_Z - 1) << CHUNK_BITS_XY;

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	ChunkCoords m_chunkPosition;
	WorldPosition m_bottomLeftCorner;
	Chunk* m_eastChunk;
	Chunk* m_westChunk;
	Chunk* m_northChunk;
	Chunk* m_southChunk;
	World* m_world;
	bool m_isDirty;

private:
	Block m_blocks[BLOCKS_PER_CHUNK];
	MeshRenderer* m_meshRenderer;
	int m_numVerts;
};

#include "Game/Chunk.inl"