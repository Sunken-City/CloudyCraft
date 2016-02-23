#pragma once
#include "Engine/Renderer/SpriteSheet.hpp"

class AABB2;

//ENUMS//////////////////////////////////////////////////////////////////////////
enum class AnimMode
{
	ONE_SHOT,	//Play from 0 to Duration, then end.
	LOOP,		//Play from 0 to Duration, then loop
	PING_PONG,
	NUM_ANIM_MODES
};

class SpriteAnim
{
public:
	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
	SpriteAnim(const SpriteSheet& sheet, float durationSeconds, AnimMode mode, int startSpriteIndex, int endSpriteIndex);
	~SpriteAnim();

	//FUNCTIONS//////////////////////////////////////////////////////////////////////////
	void Update(float deltaSeconds);
	AABB2 GetCurrentTexCoords() const;	// Based on the current elapsed time
	Texture* GetTexture() const;
	void Pause();						// Starts unpaused (playing) by default
	void Resume();						// Resume after pausing
	void Restart();						// Rewinds to time 0 and starts (re)playing
	bool IsFinished() const { return m_isFinished; }
	bool IsPlaying() const { return m_isPlaying; }

	//GETTERS//////////////////////////////////////////////////////////////////////////
	float GetSecondsRemaining() const;
	float GetFractionElapsed() const;
	float GetFractionRemaining() const;
	float GetDurationSeconds() const { return m_durationSeconds; }
	float GetSecondsElapsed() const { return m_age; }

	//SETTERS//////////////////////////////////////////////////////////////////////////
	void SetSecondsElapsed(float secondsElapsed);	    // Jump to specific time
	void SetFractionElapsed(float fractionElapsed);    // e.g. 0.33f for one-third in
	void SetDuration(float durationSeconds);

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
private:
	const SpriteSheet& m_sheet;
	AnimMode m_animationMode;
	int m_startIndex;
	int m_endIndex;
	float m_age;
	float m_secondsPerFrame;
	float m_durationSeconds;
	bool m_isPlaying;
	bool m_isFinished;
	SpriteAnim& operator=(const SpriteAnim&); //Removes a warning. This class isn't designed to be copied.
};
