#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/MathUtils.hpp"

//Built from work created by Prof. Brian "Squirrel" Eiserloh

//-----------------------------------------------------------------------------------------------
// #Eiserloh: Some of these pseudorandom noise function prime numbers are taken from various forum
//	post suggestions online.  However, experimentally they have proven to be less than stellar
//	in terms of output randomness.
// #TODO: research, or experiment, better noise functions and/or
//	noise function constants.  (Still, they're MORE than good enough for most applications, e.g.
//	Perlin noise, etc.)
//

//-----------------------------------------------------------------------------------------------
float GetPseudoRandomNoise1D( int position );
float GetPseudoRandomNoise2D( int positionX, int positionY );
float GetPseudoRandomNoise3D( int positionX, int positionY, int positionZ );
Vector2 GetPseudoRandomNoiseDirection2D( int positionX, int positionY );
float ComputePerlinNoise2D( const Vector2& position, float perlinNoiseGridCellSize, int numOctaves, float persistance );


//-----------------------------------------------------------------------------------------------
inline float GetPseudoRandomNoise1D( int position )
{
	const float ONE_OVER_MAX_POSITIVE_INT = (1.f / 2147483648.f);
	int bits = (position << 13) ^ position;
	int pseudoRandomPositiveInt = (bits * ((bits * bits * 15731) + 789221) + 1376312589) & 0x7fffffff;
	float pseudoRandomFloatZeroToOne = ONE_OVER_MAX_POSITIVE_INT * (float) pseudoRandomPositiveInt;
	return pseudoRandomFloatZeroToOne;
}


//-----------------------------------------------------------------------------------------------
inline float GetPseudoRandomNoise2D( int positionX, int positionY )
{
	const float ONE_OVER_MAX_POSITIVE_INT = (1.f / 2147483648.f);
	int position = positionX + (positionY * 57);
	int bits = (position << 13) ^ position;
	int pseudoRandomPositiveInt = (bits * ((bits * bits * 15731) + 789221) + 1376312589) & 0x7fffffff;
	float pseudoRandomFloatZeroToOne = ONE_OVER_MAX_POSITIVE_INT * (float) pseudoRandomPositiveInt;
	return pseudoRandomFloatZeroToOne;
}


//-----------------------------------------------------------------------------------------------
inline float GetPseudoRandomNoise3D( int positionX, int positionY, int positionZ )
{
	const float ONE_OVER_MAX_POSITIVE_INT = (1.f / 2147483648.f);
	int seed = positionX + (positionY * 57) + (positionZ * 3023);
	int bits = (seed << 13) ^ seed;
	int pseudoRandomPositiveInt = (bits * ((bits * bits * 15731) + 789221) + 1376312589) & 0x7fffffff;
	float pseudoRandomFloatZeroToOne = ONE_OVER_MAX_POSITIVE_INT * (float) pseudoRandomPositiveInt;
	return pseudoRandomFloatZeroToOne;
}


//-----------------------------------------------------------------------------------------------
inline Vector2 GetPseudoRandomNoiseDirection2D( int xPosition, int yPosition )
{
	const float ONE_OVER_MAX_POSITIVE_INT = (1.f / 2147483648.f);
	const float SCALE_FACTOR = ONE_OVER_MAX_POSITIVE_INT * 360.f;
	int position = xPosition + (yPosition * 57);
	int bits = (position << 13) ^ position;
	int pseudoRandomPositiveInt = (bits * ((bits * bits * 15731) + 789221) + 1376312589) & 0x7fffffff;
	float pseudoRandomDegrees = SCALE_FACTOR * (float) pseudoRandomPositiveInt;

	// #TODO: Rewrite this to use the randomized int to look up Vector2 from a (small) cos/sin
	//	table; vectors don't need to be super-precise, and we certainly don't want to pay full
	//	price for cos/sin if this is merely going to be used for, say, Perlin noise gradients.
	//	Note that cos/sin are typically fast on GPUs so this can probably stand in shader code.
	return Vector2( MathUtils::CosDegrees( pseudoRandomDegrees ), MathUtils::SinDegrees( pseudoRandomDegrees ) );
}


