#pragma once

#include "Engine/Renderer/RGBA.hpp"


//-----------------------------------------------------------------------------------------------
enum TextEffectType
{
	WAVE,
	SHAKE,
	DILATE,
	POP,
	COLOR,
	RAINBOW,
	NUM_EFFECT_TYPES
};


//-----------------------------------------------------------------------------------------------
struct TextEffect
{
	float wave = 0.f;
	bool shake = false;
	float dilate = 0.f;
	bool pop = false;
	RGBA color1 = RGBA::WHITE;
	RGBA color2 = RGBA::WHITE;
	bool rainbow = false;
};