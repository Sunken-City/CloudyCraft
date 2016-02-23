#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"

class Light
{
	enum class LightType
	{
		POINT,
		DIRECTIONAL,
		SPOTLIGHT,
		NUM_LIGHTS
	};
public:
	Light();
	~Light();
private:
	MeshRenderer m_meshRenderer;
};