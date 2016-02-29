#include "Engine/Renderer/Light.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/RGBA.hpp"

Light::Light()
{

}

Light::Light(LightType type, const Vector3& lightPosition, const RGBA& lightColor)
	: m_meshRenderer(Mesh::CreateIcoSphere(0.3f, lightColor, 2), Renderer::instance->m_defaultMaterial)
	, m_lightType(type)
	, m_lightPosition(lightPosition)
	, m_lightColor(lightColor.ToVec4())
	, m_lightDirection(Vector3::ZERO)
	, m_nearPower(0.0f)
	, m_farPower(0.0f)
	, m_nearDistance(0.0f)
	, m_farDistance(0.0f)
	, m_innerAngle(0.0f)
	, m_outerAngle(0.0f)
	, m_innerPower(0.0f)
	, m_outerPower(0.0f)
{
	m_meshRenderer.m_material->SetVec4Uniform("gColor", m_lightColor);
}

Light::~Light()
{

}

void Light::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

void Light::Render(const Matrix4x4& view, const Matrix4x4& projection) const
{
	m_meshRenderer.Render(view, projection);
}

void Light::SetPosition(const Vector3& position)
{
	m_lightPosition = position;
	m_meshRenderer.SetPosition(position);
}
