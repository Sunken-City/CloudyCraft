#pragma once
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnim.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Renderer/RGBA.hpp"

class Particle
{
public:
	Particle(const AABB2& boundingBox, const Vector2& position, float orientation, float duration, const RGBA& color, const SpriteSheet& m_spriteSheet);
	~Particle();
	virtual void Update(float deltaTime);
	virtual void Render() const;

private:
	const SpriteSheet& m_spriteSheet;
	SpriteAnim m_animation;
	AABB2 m_boundingBox;
	RGBA m_color;
	Vector2 m_position;
	Vector2 m_velocity;
	Vector2 m_acceleration;
	float m_angularVelocity;
	float m_orientation;
	float m_ageInSeconds;
	bool m_isDead;

	Particle& operator=(const Particle&);
};
