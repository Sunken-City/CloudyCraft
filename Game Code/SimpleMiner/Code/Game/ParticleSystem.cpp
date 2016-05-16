#include "Game/ParticleSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"

//---------------------------------------------------------------------------
Particle::Particle(State initialState, const RGBA& startingColor, float lifespan, float startingSize)
: m_state(initialState)
, m_startingColor(startingColor)
, m_endingColor(startingColor)
, m_lifespanInSeconds(lifespan)
, m_originalSize(startingSize)
, m_ageInSeconds(0.0f)
, m_currentSize(startingSize)
, m_isReadyToSpawn(true)
, m_isDead(false)
{

}

//---------------------------------------------------------------------------
Particle::~Particle()
{
}

//---------------------------------------------------------------------------
void Particle::Update(float deltaSeconds)
{
	m_ageInSeconds += deltaSeconds;
	if (m_ageInSeconds > m_lifespanInSeconds)
	{
		m_isDead = true;
	}
	m_state.velocity += m_state.acceleration * deltaSeconds;
	m_state.position += m_state.velocity * deltaSeconds;
}

//---------------------------------------------------------------------------
void Particle::Render() const
{
	Renderer::instance->DrawPoint(m_state.position, m_startingColor, m_currentSize);
}

//---------------------------------------------------------------------------
ParticleEmitter::ParticleEmitter(int maxNumberOfParticles, ParticleType type, float particlesPerSecond)
	: m_maxNumberParticles(maxNumberOfParticles)
	, m_particleType(type)
	, m_particlesPerSecond(particlesPerSecond)
	, m_ageInSeconds(0.0f)
	, m_currentNumberParticles(0)
{

}

//---------------------------------------------------------------------------
ParticleEmitter::~ParticleEmitter()
{
	for (Particle* particle : m_particlePool)
	{
		delete particle;
	}
}

//---------------------------------------------------------------------------
void ParticleEmitter::Update(float deltaSeconds)
{
	m_ageInSeconds += deltaSeconds;
	if (m_currentNumberParticles < m_maxNumberParticles)
	{
		//m_particlePool.emplace(new ExplosionParticle(State(), RGBA::RED, 3.0f, 5.0f));
		//m_particlePool.emplace(new FireworkParticle(State(), RGBA::FOREST_GREEN, 3.0f, 5.0f));
		//m_particlePool.emplace(new FountainParticle(State(), RGBA::BLUE, 3.0f, 5.0f));
		//m_particlePool.emplace(new SmokeParticle(State(), RGBA::GRAY, 3.0f, 5.0f));
		//m_particlePool.emplace(new DebrisParticle(State(), RGBA::CHOCOLATE, 5.0f, 5.0f));
		m_particlePool.emplace(new BubbleParticle(State(), RGBA::WHITE, 5.0f, 5.0f));
		m_currentNumberParticles++;
	}
	for (auto iter = m_particlePool.begin(); iter != m_particlePool.end(); ++iter)
	{
		Particle* particle = *iter;
		if (particle->m_isDead)
		{
			delete particle;
			iter = m_particlePool.erase(iter);
			m_currentNumberParticles--;
			if (iter == m_particlePool.end())
			{
				break;
			}
			else
			{
				continue;
			}
		}
		particle->Update(deltaSeconds);
	}
}

//---------------------------------------------------------------------------
void ParticleEmitter::Render() const
{
	for (Particle* particle : m_particlePool)
	{
		particle->Render();
	}
}

//---------------------------------------------------------------------------
ExplosionParticle::ExplosionParticle(const State& initialState, const RGBA& startingColor, float lifespan, float startingSize) : Particle(initialState, startingColor, lifespan, startingSize)
{
	m_state.velocity += Vector3(MathUtils::GetRandom(-5.0f, 5.0f), MathUtils::GetRandom(-5.0f, 5.0f), MathUtils::GetRandom(-5.0f, 5.0f));
}

//---------------------------------------------------------------------------
ExplosionParticle::~ExplosionParticle()
{

}

//---------------------------------------------------------------------------
void ExplosionParticle::Update(float deltaSeconds)
{
	Particle::Update(deltaSeconds);
}

FireworkParticle::FireworkParticle(const State& initialState, const RGBA& startingColor, float lifespan, float startingSize) : Particle(initialState, startingColor, lifespan, startingSize)
{
	m_state.velocity += Vector3(MathUtils::GetRandom(-5.0f, 5.0f), MathUtils::GetRandom(-5.0f, 5.0f), MathUtils::GetRandom(-5.0f, 7.0f));
	m_state.acceleration += Vector3(0.0f, 0.0f, -9.81f);
}

FireworkParticle::~FireworkParticle()
{

}

void FireworkParticle::Update(float deltaSeconds)
{
	Particle::Update(deltaSeconds);
}

FountainParticle::FountainParticle(const State& initialState, const RGBA& startingColor, float lifespan, float startingSize) : Particle(initialState, startingColor, lifespan, startingSize)
{
	m_state.velocity += Vector3(MathUtils::GetRandom(-1.0f, 1.0f), MathUtils::GetRandom(-1.0f, 1.0f), MathUtils::GetRandom(15.0f, 20.0f));
	m_state.acceleration += Vector3(0.0f, 0.0f, -9.81f);
}

FountainParticle::~FountainParticle()
{

}

void FountainParticle::Update(float deltaSeconds)
{
	Particle::Update(deltaSeconds);
}

SmokeParticle::SmokeParticle(const State& initialState, const RGBA& startingColor, float lifespan, float startingSize) : Particle(initialState, startingColor, lifespan, startingSize)
{
	const float dragCoefficient = 0.1f;
	m_state.velocity += Vector3(MathUtils::GetRandom(-1.0f, 1.0f), MathUtils::GetRandom(-1.0f, 1.0f), MathUtils::GetRandom(2.0f, 5.0f));
	m_state.acceleration += (m_state.velocity - Vector3(20.0f, 0.0f, 0.0f)) * dragCoefficient;
}

SmokeParticle::~SmokeParticle()
{

}

void SmokeParticle::Update(float deltaSeconds)
{
	Particle::Update(deltaSeconds);
}

DebrisParticle::DebrisParticle(const State& initialState, const RGBA& startingColor, float lifespan, float startingSize)
	: Particle(initialState, startingColor, lifespan, startingSize)
{
	m_state.velocity += Vector3(MathUtils::GetRandom(-1.0f, 1.0f), MathUtils::GetRandom(-1.0f, 1.0f), MathUtils::GetRandom(5.0f, 10.0f));
	m_state.acceleration += Vector3(0.0f, 0.0f, -9.81f);
}

DebrisParticle::~DebrisParticle()
{

}

void DebrisParticle::Update(float deltaSeconds)
{
	if (m_state.position.z < 0.0f)
	{
		m_state.position.z = 0.0f;
		m_state.velocity.z *= -1.0f;
	}
	Particle::Update(deltaSeconds);
}

BubbleParticle::BubbleParticle(const State& initialState, const RGBA& startingColor, float lifespan, float startingSize)
	: Particle(initialState, startingColor, lifespan, startingSize)
	, m_initialXVelocity(MathUtils::GetRandom(-1.0f, 1.0f))
	, m_initialYVelocity(MathUtils::GetRandom(-1.0f, 1.0f))
{
	m_state.velocity += Vector3(m_initialXVelocity, m_initialYVelocity, MathUtils::GetRandom(1.0f, 5.0f));
}

BubbleParticle::~BubbleParticle()
{

}

void BubbleParticle::Update(float deltaSeconds)
{
	m_state.velocity = Vector3(m_initialXVelocity + sin(m_ageInSeconds + m_initialYVelocity), m_initialYVelocity + sin(m_ageInSeconds + m_initialXVelocity), m_state.velocity.z);
	Particle::Update(deltaSeconds);
}
