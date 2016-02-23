#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include <set>

typedef void(*ParticlePhysicsUpdateFunctionPointer)(float deltaSeconds);

//-----------------------------------------------------------------------------------------------------------------------------------------------------
enum ParticleType
{
	EXPLOSION,
	FIREWORKS,
	SMOKE,
	FOUNTAIN,
	DEBRIS,
	NUM_PARTICLES
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------
struct State
{
	State() : position(Vector3::ZERO), velocity(Vector3::ZERO), acceleration(Vector3::ZERO) {};
	State(const Vector3& position, const Vector3& velocity, const Vector3& acceleration) : position(position), velocity(velocity), acceleration(acceleration) {};
	Vector3 position;
	Vector3 velocity;
	Vector3 acceleration;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------
class Particle
{
public:
	Particle(State initialState, const RGBA& startingColor, float lifespan, float startingSize);
	virtual ~Particle();
	virtual void Update(float deltaSeconds);
	virtual void Render() const;

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	State m_state;
	RGBA m_startingColor;
	RGBA m_endingColor;
	const float m_lifespanInSeconds;
	const float m_originalSize;
	float m_ageInSeconds;
	float m_currentSize;
	bool m_isReadyToSpawn;
	bool m_isDead;
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------
class ParticleEmitter
{
public:
	ParticleEmitter(int maxNumberOfParticles, ParticleType type, float particlesPerSecond);
	~ParticleEmitter();
	void Update(float deltaSeconds);
	void Render() const;

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
private:
	std::set<Particle*> m_particlePool;
	int m_maxNumberParticles;
	int m_currentNumberParticles;
	float m_particlesPerSecond;
	float m_ageInSeconds;
	ParticleType m_particleType;
};

//PARTICLE TYPES//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//Goes out at a random velocity, no acceleration
class ExplosionParticle : public Particle
{
public:
	ExplosionParticle(const State& initialState, const RGBA& startingColor, float lifespan, float startingSize);
	~ExplosionParticle();
	void Update(float deltaSeconds);
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//Goes out at a random velocity, gravity included
class FireworkParticle : public Particle
{
public:
	FireworkParticle(const State& initialState, const RGBA& startingColor, float lifespan, float startingSize);
	~FireworkParticle();
	void Update(float deltaSeconds);
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//Goes out at a random velocity, gravity included, cone-spread emitter
class FountainParticle : public Particle
{
public:
	FountainParticle(const State& initialState, const RGBA& startingColor, float lifespan, float startingSize);
	~FountainParticle();
	void Update(float deltaSeconds);
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//Goes out at a random velocity, wind force, cone-spread emitter
class SmokeParticle : public Particle
{
public:
	SmokeParticle(const State& initialState, const RGBA& startingColor, float lifespan, float startingSize);
	~SmokeParticle();
	void Update(float deltaSeconds);
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//Goes out at a random velocity, gravity, bounces, cone-spread emitter
class DebrisParticle : public Particle
{
public:
	DebrisParticle(const State& initialState, const RGBA& startingColor, float lifespan, float startingSize);
	~DebrisParticle();
	void Update(float deltaSeconds);
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//Goes out at a random velocity, gravity, bounces, cone-spread emitter
class BubbleParticle : public Particle
{
public:
	BubbleParticle(const State& initialState, const RGBA& startingColor, float lifespan, float startingSize);
	~BubbleParticle();
	void Update(float deltaSeconds);

	float m_initialXVelocity;
	float m_initialYVelocity;
};

