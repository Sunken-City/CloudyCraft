#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"

class Light
{
public:
	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
	Light();
	Light(const Vector3& lightPosition, const RGBA& lightColor);
	Light(const Vector3& lightPosition, const RGBA& lightColor, Material* material);
	~Light();
	void ConvertToGlobalPointLight(float nearDistance, float farDistance);
	void ConvertToLocalPointLight(float nearDistance, float farDistance, float nearPower = 1.0f, float farPower = 0.0f);
	void ConvertToLocalSpotlight(const Vector3& direction, float nearDistance, float farDistance, float innerAngle, float outerAngle, float nearPower = 1.0f, float farPower = 0.0f, float innerPower = 1.0f, float outerPower = 0.0f);
	void ConvertToGlobalSpotlight(const Vector3& direction, float nearDistance, float farDistance, float innerAngle, float outerAngle, float innerPower = 1.0f, float outerPower = 0.0f);
	void ConvertToGlobalDirectLight(const Vector3& direction, float nearDistance, float farDistance);

	//FUNCTIONS//////////////////////////////////////////////////////////////////////////
	void Update(float deltaSeconds);

	void Render() const;
	//QUERIES//////////////////////////////////////////////////////////////////////////
	bool IsDirectional();

	//SETTERS//////////////////////////////////////////////////////////////////////////
	void SetPosition(const Vector3& position);
	void SetDirection(const Vector3& direction);

	//GETTERS//////////////////////////////////////////////////////////////////////////
	inline Vector3 GetPosition() { return m_lightPosition; };
	inline Vector4 GetColor() { return m_lightColor; };
	inline Vector3 GetDirection() { return m_lightDirection; };
	inline float GetNearDistance() { return m_nearDistance; };
	inline float GetFarDistance() { return m_farDistance; };
	inline float GetInnerAngle() { return m_innerAngle; };
	inline float GetOuterAngle() { return m_outerAngle; };
	inline float GetNearPower() { return m_nearPower; };
	inline float GetFarPower() { return m_farPower; };
	inline float GetInnerPower() { return m_innerPower; };
	inline float GetOuterPower() { return m_outerPower; };

private:
	//ENUMS//////////////////////////////////////////////////////////////////////////
	enum class LightType
	{
		POINT,
		SPOTLIGHT,
		GLOBAL_POINT,
		GLOBAL_DIRECTIONAL,
		GLOBAL_SPOTLIGHT,
		NUM_LIGHTS
	};

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
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