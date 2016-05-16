#include "Engine/Renderer/Light.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Renderer/Mesh.hpp"

Light::Light()
{

}

Light::Light(const Vector3& lightPosition, const RGBA& lightColor)
	: m_meshRenderer(new Mesh()/*Mesh::CreateIcoSphere(0.3f, lightColor, 2)*/, Renderer::instance->m_defaultMaterial)
	, m_lightType(LightType::POINT)
	, m_lightPosition(lightPosition)
	, m_lightColor(lightColor.ToVec4())
	, m_lightDirection(Vector3::ZERO)
	, m_nearPower(1.0f)
	, m_farPower(1.0f)
	, m_nearDistance(2.0f)
	, m_farDistance(6.0f)
	, m_innerPower(1.0f)
	, m_outerPower(1.0f)
	, m_innerAngle(1.0f)
	, m_outerAngle(-1.0f)
{
}

Light::Light(const Vector3& lightPosition, const RGBA& lightColor, Material* material)
	: m_meshRenderer(new Mesh()/*Mesh::CreateIcoSphere(0.3f, lightColor, 2)*/, material)
	, m_lightType(LightType::POINT)
	, m_lightPosition(lightPosition)
	, m_lightColor(lightColor.ToVec4())
	, m_lightDirection(Vector3::ZERO)
	, m_nearPower(1.0f)
	, m_farPower(1.0f)
	, m_nearDistance(2.0f)
	, m_farDistance(6.0f)
	, m_innerPower(1.0f)
	, m_outerPower(1.0f)
	, m_innerAngle(1.0f)
	, m_outerAngle(-1.0f)
{

}

Light::~Light()
{

}

void Light::ConvertToGlobalPointLight(float nearDistance, float farDistance)
{
	m_lightType = LightType::GLOBAL_POINT;
	m_nearPower = 1.0f;
	m_farPower = 1.0f;
	m_nearDistance = nearDistance;
	m_farDistance = farDistance;
	m_innerAngle = 1.0f;
	m_outerAngle = -1.0f;
	m_innerPower = 1.0f;
	m_outerPower = 1.0f;
}

void Light::ConvertToLocalPointLight(float nearDistance, float farDistance, float nearPower/* = 1.0f*/, float farPower/* = 1.0f*/)
{
	m_lightType = LightType::POINT;
	m_nearPower = nearPower;
	m_farPower = farPower;
	m_nearDistance = nearDistance;
	m_farDistance = farDistance;
	m_innerAngle = 1.0f;
	m_outerAngle = -1.0f;
	m_innerPower = 1.0f;
	m_outerPower = 1.0f;
}

void Light::ConvertToLocalSpotlight(const Vector3& direction, float nearDistance, float farDistance, float innerAngle, float outerAngle, float nearPower/* = 1.0f*/, float farPower/* = 0.0f*/, float innerPower/* = 1.0f*/, float outerPower/* = 0.0f*/)
{
	m_lightType = LightType::SPOTLIGHT;
	m_nearPower = nearPower;
	m_farPower = farPower;
	m_nearDistance = nearDistance;
	m_farDistance = farDistance;
	m_innerAngle = innerAngle;
	m_outerAngle = outerAngle;
	m_innerPower = innerPower;
	m_outerPower = outerPower;
	m_lightDirection = direction;
}

void Light::ConvertToGlobalSpotlight(const Vector3& direction, float nearDistance, float farDistance, float innerAngle, float outerAngle, float innerPower/* = 1.0f*/, float outerPower/* = 0.0f*/)
{
	m_lightType = LightType::GLOBAL_SPOTLIGHT;
	m_nearPower = 1.0f;
	m_farPower = 1.0f;
	m_nearDistance = nearDistance;
	m_farDistance = farDistance;
	m_innerAngle = innerAngle;
	m_outerAngle = outerAngle;
	m_innerPower = innerPower;
	m_outerPower = outerPower;
	m_lightDirection = direction;
}

void Light::ConvertToGlobalDirectLight(const Vector3& direction, float nearDistance, float farDistance)
{
	m_lightType = LightType::GLOBAL_POINT;
	m_nearPower = 1.0f;
	m_farPower = 1.0f;
	m_nearDistance = nearDistance;
	m_farDistance = farDistance;
	m_innerAngle = 1.0f;
	m_outerAngle = -1.0f;
	m_innerPower = 1.0f;
	m_outerPower = 1.0f;
	m_lightDirection = direction;
}

void Light::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

void Light::Render() const
{
	m_meshRenderer.Render();
}

bool Light::IsDirectional()
{
	return (m_lightType == LightType::GLOBAL_DIRECTIONAL);
}

void Light::SetPosition(const Vector3& position)
{
	m_lightPosition = position;
	m_meshRenderer.SetPosition(position);
}

void Light::SetDirection(const Vector3& direction)
{
	m_lightDirection = direction;
}
