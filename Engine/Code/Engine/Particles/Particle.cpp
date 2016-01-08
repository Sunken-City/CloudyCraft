#include "Engine/Particles/Particle.hpp"
#include "Engine/Renderer/TheRenderer.hpp"

Particle::Particle(const AABB2& boundingBox, const Vector2& position, float orientation, float duration, const RGBA& color, const SpriteSheet& m_spriteSheet)
: m_spriteSheet(m_spriteSheet)
, m_animation(m_spriteSheet, duration, AnimMode::ONE_SHOT, 0, m_spriteSheet.GetNumSprites() - 1)
, m_position(position)
, m_orientation(orientation)
, m_boundingBox (boundingBox)
, m_color(color)
{

}

Particle::~Particle()
{
}

void Particle::Update(float deltaTime)
{
	m_animation.Update(deltaTime);
	if (m_animation.IsFinished())
	{
		m_isDead = true;
	}
}

void Particle::Render() const
{
	TheRenderer::instance->PushMatrix();

	TheRenderer::instance->Translate(m_position);
	TheRenderer::instance->Rotate(m_orientation);

	//Translate backwards half of the bounding box so we rotate around the center of the AABB2
	TheRenderer::instance->Translate(m_boundingBox.maxs * -0.5);
	AABB2 currentTextureCoords = m_animation.GetCurrentTexCoords();

	TheRenderer::instance->EnableAdditiveBlending();
	TheRenderer::instance->DrawTexturedAABB(m_boundingBox, currentTextureCoords.mins, currentTextureCoords.maxs, *m_animation.GetTexture(), m_color);
	TheRenderer::instance->EnableAlphaBlending();

	TheRenderer::instance->PopMatrix();
}
