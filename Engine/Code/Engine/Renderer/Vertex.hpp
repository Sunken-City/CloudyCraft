#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Renderer/RGBA.hpp"

struct Vertex_PCT
{
	Vector3 pos;
	RGBA color;
	Vector2 texCoords;
};