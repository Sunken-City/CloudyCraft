#pragma once
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector2Int.hpp"
#include "Engine/Math/Vector3Int.hpp"
#include "Engine/Renderer/TheRenderer.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Renderer/Vertex.hpp"
#include "Engine/Audio/Audio.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/ProfilingUtils.h"
#include "Game/TheGame.hpp"
#include <vector>

//ENUMS//////////////////////////////////////////////////////////////////////////
enum Direction
{
	ABOVE = 0,
	BELOW,
	NORTH,
	SOUTH,
	EAST,
	WEST,
	NUM_DIRECTIONS
};

typedef Vector3 WorldPosition; //Any free floating point or coordinate in the world (+/-)
typedef Vector3Int LocalCoords; //The block location relative to the current chunk (+)
typedef Vector3Int WorldCoords; //Global block coordinates (+/-)
typedef Vector2Int ChunkCoords; //Location of the chunk in the grid of all chunks (+/-)
typedef int LocalIndex; // (+)
typedef unsigned char uchar;

#define UNUSED(x) (void)(x);

extern bool g_isQuitting;
extern bool g_renderDebug;
extern std::vector<Vertex_PCT> g_debugPoints;
extern ProfilingID g_frameTimeProfiling;
extern ProfilingID g_updateProfiling;
extern ProfilingID g_renderProfiling;
extern int g_frameNumber;

//--------------------------------------------------------------------
inline void AddDebugPoint(const Vector3& position, const RGBA& color)
{
	Vertex_PCT vertex;
	vertex.pos = position;
	vertex.color = color;
	g_debugPoints.push_back(vertex);
}

//--------------------------------------------------------------------
inline void DrawDebugPoints(float pointSize, bool enableDepthTesting)
{
	TheRenderer::instance->EnableDepthTest(enableDepthTesting);
	TheRenderer::instance->SetPointSize(pointSize);
	TheRenderer::instance->DrawVertexArray(g_debugPoints.data(), g_debugPoints.size(), TheRenderer::DrawMode::POINTS);
}

//--------------------------------------------------------------------
inline void ClearDebugPoints()
{
	g_debugPoints.clear();
}
