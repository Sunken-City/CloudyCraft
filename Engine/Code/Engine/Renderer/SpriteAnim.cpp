#include "SpriteAnim.hpp"
#include "Engine/Renderer/AABB2.hpp"

//-----------------------------------------------------------------------------------
SpriteAnim::SpriteAnim(const SpriteSheet& sheet, float durationSeconds, AnimMode mode, int startSpriteIndex, int endSpriteIndex)
: m_sheet(sheet)
, m_durationSeconds(durationSeconds)
, m_animationMode(mode)
, m_startIndex(startSpriteIndex)
, m_endIndex(endSpriteIndex)
, m_secondsPerFrame(durationSeconds / static_cast<float>((endSpriteIndex + 1) - startSpriteIndex))
, m_age(0.f)
, m_isFinished(false)
, m_isPlaying(true)
{
	
}

//-----------------------------------------------------------------------------------
SpriteAnim::~SpriteAnim()
{
}

//-----------------------------------------------------------------------------------
void SpriteAnim::Update(float deltaSeconds)
{
	if (m_isPlaying)
	{
		m_age += deltaSeconds;
		if (m_age > m_durationSeconds)
		{
			m_isFinished = true;
		}
	}
}

//-----------------------------------------------------------------------------------
AABB2 SpriteAnim::GetCurrentTexCoords() const
{
	//Multiply m_age by fps instead lel
	int currentIndex = (int)floor(m_age / m_secondsPerFrame) + m_startIndex;
	return m_sheet.GetTexCoordsForSpriteIndex(currentIndex);
}

//-----------------------------------------------------------------------------------
Texture* SpriteAnim::GetTexture() const
{
	return m_sheet.GetTexture();
}

//-----------------------------------------------------------------------------------
void SpriteAnim::Pause()
{
	m_isPlaying = false;
}

//-----------------------------------------------------------------------------------
void SpriteAnim::Resume()
{
	m_isPlaying = true;
}

//-----------------------------------------------------------------------------------
void SpriteAnim::Restart()
{
	m_isFinished = false;
	m_age = 0.f;
}

//-----------------------------------------------------------------------------------
float SpriteAnim::GetSecondsRemaining() const
{
	return m_durationSeconds - m_age;
}

//-----------------------------------------------------------------------------------
float SpriteAnim::GetFractionElapsed() const
{
	return m_age / m_durationSeconds;
}

//-----------------------------------------------------------------------------------
float SpriteAnim::GetFractionRemaining() const
{
	return (1.f - GetFractionElapsed());
}

//-----------------------------------------------------------------------------------
void SpriteAnim::SetSecondsElapsed(float secondsElapsed)
{
	m_age = secondsElapsed;
}

//-----------------------------------------------------------------------------------
void SpriteAnim::SetFractionElapsed(float fractionElapsed)
{
	m_age = fractionElapsed * m_durationSeconds;
}

//-----------------------------------------------------------------------------------
void SpriteAnim::SetDuration(float durationSeconds)
{
	m_age = 0.0f;
	m_isFinished = false;
	m_durationSeconds = durationSeconds;
	m_secondsPerFrame = m_durationSeconds / static_cast<float>((m_endIndex + 1) - m_startIndex);
	Restart();
}
