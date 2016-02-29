#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"

class Light
{
public:
	enum class LightType
	{
		POINT,
		GLOBAL_POINT,
		DIRECTIONAL,
		SPOTLIGHT,
		NUM_LIGHTS
	};

	Light();
	Light(LightType type, const Vector3& lightPosition, const RGBA& lightColor);
	~Light();
	void Update(float deltaSeconds);
	void Render(const Matrix4x4& view, const Matrix4x4& projection) const;
	void SetPosition(const Vector3& position);
	inline Vector3 GetPosition() { return m_lightPosition; };
	inline Vector4 GetColor() { return m_lightColor; };


private:
	MeshRenderer m_meshRenderer;
	Vector3 m_lightPosition;
	Vector4 m_lightColor;
	Vector3 m_lightDirection;
	LightType m_lightType;
	float m_nearDistance;
	float m_farDistance;
	float m_innerAngle;
	float m_outerAngle;
	float m_innerPower;
	float m_outerPower;
	float m_nearPower;
	float m_farPower;
};