#include "Chunk.hpp"
#include "Engine/Math/Vector3Int.hpp"
#include "Engine/Renderer/Vertex.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/BlockDefinition.h"
#include "Game/World.hpp"
#include "Game/Generator.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include <map>

//-----------------------------------------------------------------------------------
Chunk::Chunk(const ChunkCoords& chunkCoords, World* world)
: m_chunkPosition(chunkCoords)
, m_bottomLeftCorner(WorldPosition(static_cast<float>((m_chunkPosition.x * BLOCKS_WIDE_X)), static_cast<float>((m_chunkPosition.y * BLOCKS_WIDE_Y)), 0.0f))
, m_eastChunk(nullptr)
, m_westChunk(nullptr)
, m_southChunk(nullptr)
, m_northChunk(nullptr)
, m_isDirty(false)
, m_world(world)
, m_numVerts(0)
, m_meshRenderer(nullptr)
{
    //REMINDER: THREAD-SAFE CODE ONLY!
    memset(m_blocks, 0, sizeof(m_blocks[0]) * BLOCKS_PER_CHUNK);
    GenerateChunk();
    SetEdgeBits();
}

//-----------------------------------------------------------------------------------
Chunk::Chunk(const ChunkCoords& chunkCoords, std::vector<unsigned char>& data, World* world)
: m_chunkPosition(chunkCoords)
, m_bottomLeftCorner(WorldPosition(static_cast<float>((m_chunkPosition.x * BLOCKS_WIDE_X)), static_cast<float>((m_chunkPosition.y * BLOCKS_WIDE_Y)), 0.0f))
, m_eastChunk(nullptr)
, m_westChunk(nullptr)
, m_southChunk(nullptr)
, m_northChunk(nullptr)
, m_isDirty(false)
, m_world(world)
, m_numVerts(0)
, m_meshRenderer(nullptr)
{
    //REMINDER: THREAD-SAFE CODE ONLY!
    memset(m_blocks, 0, sizeof(m_blocks[0]) * BLOCKS_PER_CHUNK);
    LoadChunkFromData(data);
    SetEdgeBits();
}

//-----------------------------------------------------------------------------------
Chunk::~Chunk()
{
    DebuggerPrintf("[%i] World [%i]: Deleting Chunk %i,%i\n", g_frameNumber, m_world->m_worldID, m_chunkPosition.x, m_chunkPosition.y);
    AttemptCleanUpRenderData();
}

//-----------------------------------------------------------------------------------
void Chunk::Update(float deltaTime)
{
    UNUSED(deltaTime);
}

//-----------------------------------------------------------------------------------
void Chunk::Render() const
{
    if (m_meshRenderer && m_meshRenderer->m_mesh->m_vbo != 0)
    {
        m_meshRenderer->Render();
    }
}

//-----------------------------------------------------------------------------------
void Chunk::GenerateChunk()
{
    StartTiming(g_generationProfiling);
    m_world->m_generator->GenerateChunk(m_blocks, this);
    EndTiming(g_generationProfiling);
}

//-----------------------------------------------------------------------------------
void Chunk::CalculateSkyLighting()
{
    //Sky Pass 1
    for (int x = 0; x < (BLOCKS_WIDE_X); x++)
    {
        for (int y = 0; y < BLOCKS_WIDE_Y; y++)
        {
            BlockInfo info = GetBlockInfoFromLocalCoords(LocalCoords(x, y, BLOCKS_TALL_Z - 1));
            while (info.m_index != BlockInfo::INVALID_INDEX)
            {
                Block* currentBlock = info.GetBlock();
                if (currentBlock->GetDefinition()->m_opacity != RGBA(0x00000000))
                {
                    info = BlockInfo::INVALID_BLOCKINFO;
                }
                else
                {
                    currentBlock->SetSky(true);
                    currentBlock->SetLightValue(m_world->m_skyLight);
                    info = info.GetBelow();
                }
            }
        }
    }
    //Sky Pass 2
    for (int x = 0; x < (BLOCKS_WIDE_X); x++)
    {
        for (int y = 0; y < BLOCKS_WIDE_Y; y++)
        {
            BlockInfo info = GetBlockInfoFromLocalCoords(LocalCoords(x, y, BLOCKS_TALL_Z - 1));
            while (info.m_index != BlockInfo::INVALID_INDEX)
            {
                Block* currentBlock = info.GetBlock();
                uchar blockType = currentBlock->m_type;
                if (BlockDefinition::GetDefinition(blockType)->m_isOpaque)
                {
                    info = BlockInfo::INVALID_BLOCKINFO;
                }
                else
                {
                    BlockInfo eastBlock = info.GetEast();
                    if (eastBlock.m_index != BlockInfo::INVALID_INDEX && !eastBlock.GetBlock()->IsSky())
                    {
                        BlockInfo::SetDirtyFlagAndAddToDirtyList(eastBlock);
                    }
                    BlockInfo westBlock = info.GetWest();
                    if (westBlock.m_index != BlockInfo::INVALID_INDEX && !westBlock.GetBlock()->IsSky())
                    {
                        BlockInfo::SetDirtyFlagAndAddToDirtyList(westBlock);
                    }
                    BlockInfo northBlock = info.GetNorth();
                    if (northBlock.m_index != BlockInfo::INVALID_INDEX && !northBlock.GetBlock()->IsSky())
                    {
                        BlockInfo::SetDirtyFlagAndAddToDirtyList(northBlock);
                    }
                    BlockInfo southBlock = info.GetSouth();
                    if (southBlock.m_index != BlockInfo::INVALID_INDEX && !southBlock.GetBlock()->IsSky())
                    {
                        BlockInfo::SetDirtyFlagAndAddToDirtyList(southBlock);
                    }
                    info = info.GetBelow();
                }
            }
        }
    }

    for (int i = 0; i < BLOCKS_PER_CHUNK; i++)
    {
        Block* currentBlock = GetBlock(i);
        BlockDefinition* definition = BlockDefinition::GetDefinition(currentBlock->m_type);
        if (definition->m_illumination > 0)
        {
            SetBlockDirtyAndAddToDirtyList(i);
        }
    }
}

//-----------------------------------------------------------------------------------
WorldCoords Chunk::GetWorldCoordsForBlockIndex(LocalIndex index) const
{
    LocalCoords coords = GetLocalCoordsFromBlockIndex(index);
    return WorldCoords(coords.x + static_cast<int>(m_bottomLeftCorner.x), coords.y + static_cast<int>(m_bottomLeftCorner.y), coords.z);
}

//-----------------------------------------------------------------------------------
LocalIndex Chunk::GetBlockIndexFromWorldCoords(const WorldCoords& coords) const
{
    LocalCoords localCoords = GetLocalCoordsFromWorldCoords(coords);
    return GetBlockIndexFromLocalCoords(localCoords);
}

//-----------------------------------------------------------------------------------
LocalCoords Chunk::GetLocalCoordsFromWorldCoords(const WorldCoords &coords) const
{
    LocalCoords localCoords = LocalCoords(coords.x - static_cast<int>(m_bottomLeftCorner.x), coords.y - static_cast<int>(m_bottomLeftCorner.y), coords.z);
    if (localCoords.z > BLOCKS_TALL_Z)
    {
        localCoords.z = BLOCKS_TALL_Z;
    }
    else if (localCoords.z < 0)
    {
        localCoords.z = 0;
    }
    return localCoords; 
}

//-----------------------------------------------------------------------------------
LocalCoords Chunk::GetLocalCoordsFromWorldPosition(const WorldPosition &pos) const
{
    return GetLocalCoordsFromWorldCoords(WorldCoords(static_cast<int>(floor(pos.x)), static_cast<int>(floor(pos.y)), static_cast<int>(floor(pos.z))));
}

//-----------------------------------------------------------------------------------
LocalIndex Chunk::GetBlockIndexFromWorldPosition(const WorldPosition& pos) const
{
    return GetBlockIndexFromWorldCoords(WorldCoords(static_cast<int>(floor(pos.x)), static_cast<int>(floor(pos.y)), static_cast<int>(floor(pos.z))));
}

//-----------------------------------------------------------------------------------
bool Chunk::IsInFrustum(const Vector3& cameraXYZ, const WorldPosition& playerPosition) const
{
    const Vector3 chunkMins = Vector3(static_cast<float>(m_chunkPosition.x * Chunk::BLOCKS_WIDE_X), static_cast<float>(m_chunkPosition.y * Chunk::BLOCKS_WIDE_Y), 0.0f);
    const Vector3 chunkOffsetX = Vector3::UNIT_X * static_cast<float>(Chunk::BLOCKS_WIDE_X);
    const Vector3 chunkOffsetY = Vector3::UNIT_Y * static_cast<float>(Chunk::BLOCKS_WIDE_Y);
    const Vector3 chunkOffsetZ = Vector3::UNIT_Z * static_cast<float>(Chunk::BLOCKS_TALL_Z);
    const Vector3 chunkMaxs = Vector3(static_cast<float>(m_chunkPosition.x * Chunk::BLOCKS_WIDE_X), static_cast<float>(m_chunkPosition.y * Chunk::BLOCKS_WIDE_Y), static_cast<float>(Chunk::BLOCKS_TALL_Z));
    if (MathUtils::Dot(cameraXYZ, chunkMins - playerPosition) > 0.0f)
    {
        return true;
    }
    else if (MathUtils::Dot(cameraXYZ, chunkMins + chunkOffsetX - playerPosition) > 0.0f)
    {
        return true;
    }
    else if (MathUtils::Dot(cameraXYZ, chunkMins + chunkOffsetY - playerPosition) > 0.0f)
    {
        return true;
    }
    else if (MathUtils::Dot(cameraXYZ, chunkMins + chunkOffsetX + chunkOffsetY - playerPosition) > 0.0f)
    {
        return true;
    }
    else if (MathUtils::Dot(cameraXYZ, chunkMins + chunkOffsetZ - playerPosition) > 0.0f)
    {
        return true;
    }
    else if (MathUtils::Dot(cameraXYZ, chunkMins + chunkOffsetX + chunkOffsetZ - playerPosition) > 0.0f)
    {
        return true;
    }
    else if (MathUtils::Dot(cameraXYZ, chunkMins + chunkOffsetY + chunkOffsetZ - playerPosition) > 0.0f)
    {
        return true;
    }
    else if (MathUtils::Dot(cameraXYZ, chunkMaxs - playerPosition) > 0.0f)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//-----------------------------------------------------------------------------------
void Chunk::GenerateVertexArray()
{
    DebuggerPrintf("[%i] World [%i]: Building Chunk %i,%i VA\n", g_frameNumber, m_world->m_worldID, m_chunkPosition.x, m_chunkPosition.y);
    StartTiming(g_vaBuildingProfiling);
    AttemptCleanUpRenderData();
    MeshBuilder builder = MeshBuilder();
    builder.Begin();
    const float blockSize = 1.0f;
    int lastIndex = 0;
    for (int i = 0; i < BLOCKS_PER_CHUNK; i++)
    {
        Block currentBlock = m_blocks[i];
        if (!currentBlock.GetDefinition()->m_isOpaque)
        {
            continue;
        }

        WorldPosition coords = GetWorldMinsForBlockIndex(i);
        Vertex_PCT vertex;
        vertex.color = RGBA(0x000000FF);
        BlockDefinition* currentDefinition = BlockDefinition::GetDefinition(currentBlock.m_type);
        AABB2 topTex = currentDefinition->GetTopIndex();
        AABB2 sideTex = currentDefinition->GetSideIndex();
        AABB2 bottomTex = currentDefinition->GetBottomIndex();
        AABB2 portalTex = TheGame::instance->m_blockSheet->GetTexCoordsForSpriteIndex(0x50);
        static const float uvStepSize = bottomTex.maxs.x - bottomTex.mins.x;

        Block* belowBlock = GetBelow(i);
        if (belowBlock && (!BlockDefinition::GetDefinition(belowBlock->m_type)->m_isOpaque || currentBlock.HasBelowPortal() || belowBlock->HasAbovePortal()))
        {
            float isPortal = currentBlock.HasBelowPortal() ? 1.0f : 0.0f;
            AABB2& textureCoords = bottomTex;
            builder.SetColor(RGBA(belowBlock->GetDampedLightValue(0x33)));
            builder.SetFloatData0(Vector4(isPortal, 0.0f, 0.0f, 0.0f));
            builder.SetUV(textureCoords.mins);
            builder.AddVertex(Vector3(coords.x, coords.y, coords.z));
            builder.SetUV(Vector2(textureCoords.maxs.x, textureCoords.mins.y));
            builder.AddVertex(Vector3(coords.x, coords.y + blockSize, coords.z));
            builder.SetUV(textureCoords.maxs);
            builder.AddVertex(Vector3(coords.x + blockSize, coords.y + blockSize, coords.z));
            builder.SetUV(Vector2(textureCoords.mins.x, textureCoords.maxs.y));
            builder.AddVertex(Vector3(coords.x + blockSize, coords.y, coords.z));
            builder.AddQuadIndicesClockwise(lastIndex + 3, lastIndex + 2, lastIndex + 0, lastIndex + 1);
            lastIndex += 4;
        }

        Block* aboveBlock = GetAbove(i);
        if (aboveBlock && (!BlockDefinition::GetDefinition(aboveBlock->m_type)->m_isOpaque || currentBlock.HasAbovePortal() || aboveBlock->HasBelowPortal()))
        {
            float isPortal = currentBlock.HasAbovePortal() ? 1.0f : 0.0f;
            AABB2& textureCoords = topTex;
            builder.SetColor(RGBA(aboveBlock->GetDampedLightValue(0x00)));
            builder.SetFloatData0(Vector4(isPortal, 0.0f, 0.0f, 0.0f));
            builder.SetUV(Vector2(textureCoords.mins.x, textureCoords.mins.y));
            builder.AddVertex(Vector3(coords.x, coords.y, coords.z + blockSize));
            builder.SetUV(Vector2(textureCoords.maxs.x, textureCoords.mins.y));
            builder.AddVertex(Vector3(coords.x + blockSize, coords.y, coords.z + blockSize));
            builder.SetUV(Vector2(textureCoords.maxs.x, textureCoords.maxs.y));
            builder.AddVertex(Vector3(coords.x + blockSize, coords.y + blockSize, coords.z + blockSize));
            builder.SetUV(Vector2(textureCoords.mins.x, textureCoords.maxs.y));
            builder.AddVertex(Vector3(coords.x, coords.y + blockSize, coords.z + blockSize));
            builder.AddQuadIndicesClockwise(lastIndex + 3, lastIndex + 2, lastIndex + 0, lastIndex + 1);
            lastIndex += 4;
        }

        Block* westBlock = GetWest(i);
        if (westBlock && (!BlockDefinition::GetDefinition(westBlock->m_type)->m_isOpaque || currentBlock.HasWestPortal() || westBlock->HasEastPortal()))
        {
            float isPortal = currentBlock.HasWestPortal() ? 1.0f : 0.0f;
            AABB2& textureCoords = sideTex;
            builder.SetColor(RGBA(westBlock->GetDampedLightValue(0x22)));
            builder.SetFloatData0(Vector4(isPortal, 0.0f, 0.0f, 0.0f));
            builder.SetUV(Vector2(textureCoords.mins.x, textureCoords.mins.y));
            builder.AddVertex(Vector3(coords.x, coords.y + blockSize, coords.z));
            builder.SetUV(Vector2(textureCoords.maxs.x, textureCoords.mins.y));
            builder.AddVertex(Vector3(coords.x, coords.y, coords.z));
            builder.SetUV(Vector2(textureCoords.maxs.x, textureCoords.maxs.y));
            builder.AddVertex(Vector3(coords.x, coords.y, coords.z + blockSize));
            builder.SetUV(Vector2(textureCoords.mins.x, textureCoords.maxs.y));
            builder.AddVertex(Vector3(coords.x, coords.y + blockSize, coords.z + blockSize));
            builder.AddQuadIndicesClockwise(lastIndex + 3, lastIndex + 2, lastIndex + 0, lastIndex + 1);
            lastIndex += 4;
        }

        Block* eastBlock = GetEast(i);
        if (eastBlock && (!BlockDefinition::GetDefinition(eastBlock->m_type)->m_isOpaque || currentBlock.HasEastPortal() || eastBlock->HasWestPortal()))
        {
            float isPortal = currentBlock.HasEastPortal() ? 1.0f : 0.0f;
            AABB2& textureCoords = sideTex;
            builder.SetColor(RGBA(eastBlock->GetDampedLightValue(0x22)));
            builder.SetFloatData0(Vector4(isPortal, 0.0f, 0.0f, 0.0f));
            builder.SetUV(Vector2(textureCoords.mins.x, textureCoords.mins.y));
            builder.AddVertex(Vector3(coords.x + blockSize, coords.y, coords.z));
            builder.SetUV(Vector2(textureCoords.maxs.x, textureCoords.mins.y));
            builder.AddVertex(Vector3(coords.x + blockSize, coords.y + blockSize, coords.z));
            builder.SetUV(Vector2(textureCoords.maxs.x, textureCoords.maxs.y));
            builder.AddVertex(Vector3(coords.x + blockSize, coords.y + blockSize, coords.z + blockSize));
            builder.SetUV(Vector2(textureCoords.mins.x, textureCoords.maxs.y));
            builder.AddVertex(Vector3(coords.x + blockSize, coords.y, coords.z + blockSize));
            builder.AddQuadIndicesClockwise(lastIndex + 3, lastIndex + 2, lastIndex + 0, lastIndex + 1);
            lastIndex += 4;
        }

        Block* southBlock = GetSouth(i);
        if (southBlock && (!BlockDefinition::GetDefinition(southBlock->m_type)->m_isOpaque || currentBlock.HasSouthPortal() || southBlock->HasNorthPortal()))
        {
            float isPortal = currentBlock.HasSouthPortal() ? 1.0f : 0.0f;
            AABB2& textureCoords = sideTex;
            builder.SetColor(RGBA(southBlock->GetDampedLightValue(0x11)));
            builder.SetFloatData0(Vector4(isPortal, 0.0f, 0.0f, 0.0f));
            builder.SetUV(Vector2(textureCoords.mins.x, textureCoords.mins.y));
            builder.AddVertex(Vector3(coords.x, coords.y, coords.z));
            builder.SetUV(Vector2(textureCoords.maxs.x, textureCoords.mins.y));
            builder.AddVertex(Vector3(coords.x + blockSize, coords.y, coords.z));
            builder.SetUV(Vector2(textureCoords.maxs.x, textureCoords.maxs.y));
            builder.AddVertex(Vector3(coords.x + blockSize, coords.y, coords.z + blockSize));
            builder.SetUV(Vector2(textureCoords.mins.x, textureCoords.maxs.y));
            builder.AddVertex(Vector3(coords.x, coords.y, coords.z + blockSize));
            builder.AddQuadIndicesClockwise(lastIndex + 3, lastIndex + 2, lastIndex + 0, lastIndex + 1);
            lastIndex += 4;
        }

        Block* northBlock = GetNorth(i);
        if (northBlock && (!BlockDefinition::GetDefinition(northBlock->m_type)->m_isOpaque || currentBlock.HasNorthPortal() || northBlock->HasSouthPortal()))
        {
            float isPortal = currentBlock.HasNorthPortal() ? 1.0f : 0.0f;
            AABB2& textureCoords = sideTex;
            builder.SetColor(RGBA(northBlock->GetDampedLightValue(0x11)));
            builder.SetFloatData0(Vector4(isPortal, 0.0f, 0.0f, 0.0f));
            builder.SetUV(Vector2(textureCoords.mins.x, textureCoords.mins.y));
            builder.AddVertex(Vector3(coords.x + blockSize, coords.y + blockSize, coords.z));
            builder.SetUV(Vector2(textureCoords.maxs.x, textureCoords.mins.y));
            builder.AddVertex(Vector3(coords.x, coords.y + blockSize, coords.z));
            builder.SetUV(Vector2(textureCoords.maxs.x, textureCoords.maxs.y));
            builder.AddVertex(Vector3(coords.x, coords.y + blockSize, coords.z + blockSize));
            builder.SetUV(Vector2(textureCoords.mins.x, textureCoords.maxs.y));
            builder.AddVertex(Vector3(coords.x + blockSize, coords.y + blockSize, coords.z + blockSize));
            builder.AddQuadIndicesClockwise(lastIndex + 3, lastIndex + 2, lastIndex + 0, lastIndex + 1);
            lastIndex += 4;
        }
    }

    //Transparent drawing
    for (int i = 0; i < BLOCKS_PER_CHUNK; i++)
    {
        Block currentBlock = m_blocks[i];
        if (!currentBlock.IsPortal(NUM_DIRECTIONS) && (currentBlock.GetDefinition()->m_isOpaque || currentBlock.m_type == BlockType::AIR))
        {
            continue;
        }

        WorldPosition coords = GetWorldMinsForBlockIndex(i);
        Vertex_PCT vertex;
        vertex.color = RGBA(0x000000FF);
        BlockDefinition* currentDefinition = currentBlock.GetDefinition();
        AABB2 topTex = currentDefinition->GetTopIndex();
        AABB2 sideTex = currentDefinition->GetSideIndex();
        AABB2 bottomTex = currentDefinition->GetBottomIndex();

        BlockInfo info = GetBlockInfoFromLocalCoords(GetLocalCoordsFromBlockIndex(i));

        BlockInfo belowInfo = info.GetBelow();
        if (belowInfo.IsValid())
        {
            Block* belowBlock = belowInfo.GetBlock();
            BlockDefinition* belowType = belowBlock->GetDefinition();
            if (belowBlock && ((!belowType->m_isOpaque && belowBlock->m_type != currentBlock.m_type) || currentBlock.HasBelowPortal()))
            {
                float isPortal = currentBlock.HasBelowPortal() ? 1.0f : 0.0f;
                builder.SetFloatData0(Vector4(isPortal, 0.0f, 0.0f, 0.0f));
                builder.SetColor(RGBA(belowBlock->GetDampedLightValue(0x33)));
                builder.SetUV(bottomTex.mins);
                builder.AddVertex(Vector3(coords.x, coords.y, coords.z));
                builder.SetUV(Vector2(bottomTex.maxs.x, bottomTex.mins.y));
                builder.AddVertex(Vector3(coords.x, coords.y + blockSize, coords.z));
                builder.SetUV(bottomTex.maxs);
                builder.AddVertex(Vector3(coords.x + blockSize, coords.y + blockSize, coords.z));
                builder.SetUV(Vector2(bottomTex.mins.x, bottomTex.maxs.y));
                builder.AddVertex(Vector3(coords.x + blockSize, coords.y, coords.z));
                builder.AddQuadIndicesClockwise(lastIndex + 3, lastIndex + 2, lastIndex + 0, lastIndex + 1);
                lastIndex += 4;
            }
        }

        BlockInfo aboveInfo = info.GetAbove();
        if (aboveInfo.IsValid())
        {
            Block* aboveBlock = aboveInfo.GetBlock();
            BlockDefinition* aboveType = aboveBlock->GetDefinition();
            if (aboveBlock && ((!aboveType->m_isOpaque && aboveBlock->m_type != currentBlock.m_type) || currentBlock.HasAbovePortal()))
            {
                float isPortal = currentBlock.HasAbovePortal() ? 1.0f : 0.0f;
                builder.SetFloatData0(Vector4(isPortal, 0.0f, 0.0f, 0.0f));
                builder.SetColor(RGBA(aboveBlock->GetDampedLightValue(0x00)));
                builder.SetUV(Vector2(topTex.mins.x, topTex.mins.y));
                builder.AddVertex(Vector3(coords.x, coords.y, coords.z + blockSize));
                builder.SetUV(Vector2(topTex.maxs.x, topTex.mins.y));
                builder.AddVertex(Vector3(coords.x + blockSize, coords.y, coords.z + blockSize));
                builder.SetUV(Vector2(topTex.maxs.x, topTex.maxs.y));
                builder.AddVertex(Vector3(coords.x + blockSize, coords.y + blockSize, coords.z + blockSize));
                builder.SetUV(Vector2(topTex.mins.x, topTex.maxs.y));
                builder.AddVertex(Vector3(coords.x, coords.y + blockSize, coords.z + blockSize));
                builder.AddQuadIndicesClockwise(lastIndex + 3, lastIndex + 2, lastIndex + 0, lastIndex + 1);
                lastIndex += 4;
            }
        }

        BlockInfo westInfo = info.GetWest();
        if (westInfo.IsValid())
        {
            Block* westBlock = westInfo.GetBlock();
            BlockDefinition* westType = westBlock->GetDefinition();
            if (westBlock && ((!westType->m_isOpaque && westBlock->m_type != currentBlock.m_type) || currentBlock.HasWestPortal()))
            {
                float isPortal = currentBlock.HasWestPortal() ? 1.0f : 0.0f;
                builder.SetFloatData0(Vector4(isPortal, 0.0f, 0.0f, 0.0f));
                builder.SetColor(RGBA(westBlock->GetDampedLightValue(0x22)));
                builder.SetUV(Vector2(sideTex.mins.x, sideTex.mins.y));
                builder.AddVertex(Vector3(coords.x, coords.y + blockSize, coords.z));
                builder.SetUV(Vector2(sideTex.maxs.x, sideTex.mins.y));
                builder.AddVertex(Vector3(coords.x, coords.y, coords.z));
                builder.SetUV(Vector2(sideTex.maxs.x, sideTex.maxs.y));
                builder.AddVertex(Vector3(coords.x, coords.y, coords.z + blockSize));
                builder.SetUV(Vector2(sideTex.mins.x, sideTex.maxs.y));
                builder.AddVertex(Vector3(coords.x, coords.y + blockSize, coords.z + blockSize));
                builder.AddQuadIndicesClockwise(lastIndex + 3, lastIndex + 2, lastIndex + 0, lastIndex + 1);
                lastIndex += 4;
            }
        }

        BlockInfo eastInfo = info.GetEast();
        if (eastInfo.IsValid())
        {
            Block* eastBlock = eastInfo.GetBlock();
            BlockDefinition* eastType = eastBlock->GetDefinition();
            if (eastBlock && ((!eastType->m_isOpaque && eastBlock->m_type != currentBlock.m_type) || currentBlock.HasEastPortal()))
            {
                float isPortal = currentBlock.HasEastPortal() ? 1.0f : 0.0f;
                builder.SetFloatData0(Vector4(isPortal, 0.0f, 0.0f, 0.0f));
                builder.SetColor(RGBA(eastBlock->GetDampedLightValue(0x22)));
                builder.SetUV(Vector2(sideTex.mins.x, sideTex.mins.y));
                builder.AddVertex(Vector3(coords.x + blockSize, coords.y, coords.z));
                builder.SetUV(Vector2(sideTex.maxs.x, sideTex.mins.y));
                builder.AddVertex(Vector3(coords.x + blockSize, coords.y + blockSize, coords.z));
                builder.SetUV(Vector2(sideTex.maxs.x, sideTex.maxs.y));
                builder.AddVertex(Vector3(coords.x + blockSize, coords.y + blockSize, coords.z + blockSize));
                builder.SetUV(Vector2(sideTex.mins.x, sideTex.maxs.y));
                builder.AddVertex(Vector3(coords.x + blockSize, coords.y, coords.z + blockSize));
                builder.AddQuadIndicesClockwise(lastIndex + 3, lastIndex + 2, lastIndex + 0, lastIndex + 1);
                lastIndex += 4;
            }
        }

        BlockInfo southInfo = info.GetSouth();
        if (southInfo.IsValid())
        {
            Block* southBlock = southInfo.GetBlock();
            BlockDefinition* southType = southBlock->GetDefinition();
            if (southBlock && ((!southType->m_isOpaque && southBlock->m_type != currentBlock.m_type) || currentBlock.HasSouthPortal()))
            {
                float isPortal = currentBlock.HasSouthPortal() ? 1.0f : 0.0f;
                builder.SetFloatData0(Vector4(isPortal, 0.0f, 0.0f, 0.0f));
                builder.SetColor(RGBA(southBlock->GetDampedLightValue(0x11)));
                builder.SetUV(Vector2(sideTex.mins.x, sideTex.mins.y));
                builder.AddVertex(Vector3(coords.x, coords.y, coords.z));
                builder.SetUV(Vector2(sideTex.maxs.x, sideTex.mins.y));
                builder.AddVertex(Vector3(coords.x + blockSize, coords.y, coords.z));
                builder.SetUV(Vector2(sideTex.maxs.x, sideTex.maxs.y));
                builder.AddVertex(Vector3(coords.x + blockSize, coords.y, coords.z + blockSize));
                builder.SetUV(Vector2(sideTex.mins.x, sideTex.maxs.y));
                builder.AddVertex(Vector3(coords.x, coords.y, coords.z + blockSize));
                builder.AddQuadIndicesClockwise(lastIndex + 3, lastIndex + 2, lastIndex + 0, lastIndex + 1);
                lastIndex += 4;
            }
        }

        BlockInfo northInfo = info.GetNorth();
        if (northInfo.IsValid())
        {
            Block* northBlock = northInfo.GetBlock();
            BlockDefinition* northType = northBlock->GetDefinition();
            if (northBlock && ((!northType->m_isOpaque && northBlock->m_type != currentBlock.m_type) || currentBlock.HasNorthPortal()))
            {
                float isPortal = currentBlock.HasNorthPortal() ? 1.0f : 0.0f;
                builder.SetFloatData0(Vector4(isPortal, 0.0f, 0.0f, 0.0f));
                builder.SetColor(RGBA(northBlock->GetDampedLightValue(0x11)));
                builder.SetUV(Vector2(sideTex.mins.x, sideTex.mins.y));
                builder.AddVertex(Vector3(coords.x + blockSize, coords.y + blockSize, coords.z));
                builder.SetUV(Vector2(sideTex.maxs.x, sideTex.mins.y));
                builder.AddVertex(Vector3(coords.x, coords.y + blockSize, coords.z));
                builder.SetUV(Vector2(sideTex.maxs.x, sideTex.maxs.y));
                builder.AddVertex(Vector3(coords.x, coords.y + blockSize, coords.z + blockSize));
                builder.SetUV(Vector2(sideTex.mins.x, sideTex.maxs.y));
                builder.AddVertex(Vector3(coords.x + blockSize, coords.y + blockSize, coords.z + blockSize));
                builder.AddQuadIndicesClockwise(lastIndex + 3, lastIndex + 2, lastIndex + 0, lastIndex + 1);
                lastIndex += 4;
            }
        }
    }
    builder.End();
    Mesh* mesh = new Mesh();
    m_meshRenderer = new MeshRenderer(mesh, TheGame::instance->m_blockMaterial);
    builder.CopyToMesh(m_meshRenderer->m_mesh, &Vertex_PCTD::Copy, sizeof(Vertex_PCTD), &Vertex_PCTD::BindMeshToVAO);
    m_isDirty = false;
    EndTiming(g_vaBuildingProfiling);
}

//-----------------------------------------------------------------------------------
void Chunk::GenerateSaveData(std::vector<unsigned char>& data)
{
    uchar currentType = m_blocks[0].m_type;
    uchar numOfType = 0;
    data.push_back(currentType);
    for (Block block : m_blocks)
    {
        if (block.m_type == currentType && numOfType < 255)
        {
            ++numOfType;
        }
        else
        {
            data.push_back(numOfType);
            currentType = block.m_type;
            data.push_back(currentType);
            numOfType = 1;
        }
    }
    data.push_back(numOfType);
}

//-----------------------------------------------------------------------------------
void Chunk::LoadChunkFromData(std::vector<unsigned char>& data)
{
    int currentIndex = 0;
    for (unsigned int i = 0; i < data.size(); i += 2)
    {
        uchar blockType = data[i];
        int numBlocks = data[i + 1];
        for (int j = 0; j < numBlocks; j++)
        {
            m_blocks[currentIndex++].m_type = blockType;
        }
    }
}

//-----------------------------------------------------------------------------------
void Chunk::AttemptCleanUpRenderData()
{
    if (m_meshRenderer)
    {
        delete m_meshRenderer->m_mesh;
        delete m_meshRenderer;
        m_meshRenderer = nullptr;
    }
}

//-----------------------------------------------------------------------------------
void Chunk::SetBlockDirtyAndAddToDirtyList(LocalIndex blockToDirtyIndex)
{
    Block* blockToDirty = GetBlock(blockToDirtyIndex);
    blockToDirty->SetDirty(true);
    m_world->m_dirtyBlocks.emplace_back(BlockInfo(this, blockToDirtyIndex));
}

//-----------------------------------------------------------------------------------
void Chunk::DirtyAndAddToDirtyList()
{
    //Don't do anything if we're already in the list.
    if (m_isDirty == true)
    {
        return;
    }
    m_isDirty = true;
    m_world->m_dirtyChunks.emplace(this, m_world->DistanceSquaredFromPlayerToChunk(this->m_chunkPosition));
}

//-----------------------------------------------------------------------------------
void Chunk::SetHighPriorityChunkDirtyAndAddToDirtyList()
{
    m_isDirty = true;
    m_world->m_dirtyChunks.emplace(this, 0.0f);
}

//-----------------------------------------------------------------------------------
void Chunk::SetEdgeBits()
{
    //Directions are relative to the top layer of the chunk, viewed from above.
    const int TOP_LEFT_INDEX = BLOCKS_PER_CHUNK - BLOCKS_WIDE_X;
    const int TOP_RIGHT_INDEX = BLOCKS_PER_CHUNK - 1;
    const int BOTTOM_RIGHT_INDEX_MINUS_1 = BLOCKS_PER_CHUNK - BLOCKS_PER_LAYER + (BLOCKS_WIDE_X - 2);
    const int TOP_RIGHT_INDEX_MINUS_1 = BLOCKS_PER_CHUNK - 2;
    //West Side
    for (int index = 0; index < TOP_LEFT_INDEX; index += BLOCKS_WIDE_Y)
    {
        GetBlock(index)->SetEdgeBlock(true);
    }
    //East Side
    for (int index = BLOCKS_WIDE_X - 1; index < TOP_RIGHT_INDEX; index += BLOCKS_WIDE_Y)
    {
        GetBlock(index)->SetEdgeBlock(true);
    }
    //North Side
    for (int index = BLOCKS_PER_LAYER - BLOCKS_WIDE_X + 1; index < TOP_RIGHT_INDEX_MINUS_1; index += BLOCKS_PER_LAYER)
    {
        const int BLOCKS_PER_ITERATION = BLOCKS_WIDE_X - 2;
        for (int j = 0; j < BLOCKS_PER_ITERATION; j++)
        {
            GetBlock(index + j)->SetEdgeBlock(true);
        }
    }
    //South Side
    for (int index = 1; index < BOTTOM_RIGHT_INDEX_MINUS_1; index += BLOCKS_PER_LAYER)
    {
        const int BLOCKS_PER_ITERATION = BLOCKS_WIDE_X - 2;
        for (int j = 0; j < BLOCKS_PER_ITERATION; j++)
        {
            GetBlock(index + j)->SetEdgeBlock(true);
        }
    }
}

//-----------------------------------------------------------------------------------
void Chunk::FlagEdgesAsDirtyLighting(Direction dir)
{
    //Directions are relative to the top layer of the chunk, viewed from above.
    const int TOP_LEFT_INDEX = BLOCKS_PER_CHUNK - BLOCKS_WIDE_X;
    const int TOP_RIGHT_INDEX = BLOCKS_PER_CHUNK - 1;
    const int BOTTOM_RIGHT_INDEX_MINUS_1 = BLOCKS_PER_CHUNK - BLOCKS_PER_LAYER + (BLOCKS_WIDE_X - 2);
    const int TOP_RIGHT_INDEX_MINUS_1 = BLOCKS_PER_CHUNK - 2;
    switch (dir)
    {
    case NORTH:
        //North Side
        for (int index = BLOCKS_PER_LAYER - BLOCKS_WIDE_X + 1; index < TOP_RIGHT_INDEX_MINUS_1; index += BLOCKS_PER_LAYER)
        {
            const int BLOCKS_PER_ITERATION = BLOCKS_WIDE_X - 2;
            for (int j = 0; j < BLOCKS_PER_ITERATION; j++)
            {
                Block* block = GetBlock(index + j);
                if (block->IsSky() || BlockDefinition::GetDefinition(block->m_type)->IsIlluminated())
                {
                    SetBlockDirtyAndAddToDirtyList(index + j);
                }
            }
        }
        break;
    case SOUTH:
        //South Side
        for (int index = 1; index < BOTTOM_RIGHT_INDEX_MINUS_1; index += BLOCKS_PER_LAYER)
        {
            const int BLOCKS_PER_ITERATION = BLOCKS_WIDE_X - 2;
            for (int j = 0; j < BLOCKS_PER_ITERATION; j++)
            {
                Block* block = GetBlock(index + j);
                if (block->IsSky() || BlockDefinition::GetDefinition(block->m_type)->IsIlluminated())
                {
                    SetBlockDirtyAndAddToDirtyList(index + j);
                }
            }
        }
        break;
    case EAST:
        //East Side
        for (int index = BLOCKS_WIDE_X - 1; index < TOP_RIGHT_INDEX; index += BLOCKS_WIDE_Y)
        {
            Block* block = GetBlock(index);
            if (block->IsSky() || BlockDefinition::GetDefinition(block->m_type)->IsIlluminated())
            {
                SetBlockDirtyAndAddToDirtyList(index);
            }
        }
        break;
    case WEST:
        //West Side
        for (int index = 0; index < TOP_LEFT_INDEX; index += BLOCKS_WIDE_Y)
        {
            Block* block = GetBlock(index);
            if (block->IsSky() || BlockDefinition::GetDefinition(block->m_type)->IsIlluminated())
            {
                SetBlockDirtyAndAddToDirtyList(index);
            }
        }
        break;
    default:
        break;
    }
}