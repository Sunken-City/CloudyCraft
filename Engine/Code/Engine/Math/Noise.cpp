#include "Engine/Math/Noise.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector2Int.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <math.h>

//Built from work created by Prof. Brian "Squirrel" Eiserloh

//-----------------------------------------------------------------------------------------------
// Computes a random Perlin noise value in the range [-1,1] based on a 2D input <position> and
//	various Perlin noise parameters.
//
//	<perlinNoiseGridCellSize>: Noise density.  Larger values produce longer wavelength noise
//		(e.g. gentle, sweeping hills).
//	<numOctaves>: 0 is flat, 1 is simple smoothed noise. Values of 2+ add one or more additional
//		"octave" harmonics.  Each additional octave has double the frequency/density but only a
//		fraction of the amplitude of the base noise.
//	<persistence>: The fraction of amplitude of each subsequent octave, based on the amplitude of
//		the previous octave.  For example, with a persistence of 0.3, each octave is only 30% as
//		strong as the previous octave.
//
float ComputePerlinNoise2D( const Vector2& position, float perlinNoiseGridCellSize, int numOctaves, float persistence )
{
	float totalPerlinNoise = 0.f;
	float currentOctaveAmplitude = 1.f;
	float totalMaxAmplitude = 0.f;
	float perlinGridFrequency = (1.f / perlinNoiseGridCellSize);
	for( int octaveNumber = 0; octaveNumber < numOctaves; ++octaveNumber )
	{
		Vector2 perlinPosition( position.x * perlinGridFrequency, position.y * perlinGridFrequency );
		Vector2 perlinPositionFloor( (float) floor( perlinPosition.x ), (float) floor( perlinPosition.y ) );
		Vector2Int perlinCell( (int) perlinPositionFloor.x, (int) perlinPositionFloor.y );
		Vector2 perlinPositionUV = perlinPosition - perlinPositionFloor;
		Vector2 perlinPositionAntiUV( perlinPositionUV.x - 1.f, perlinPositionUV.y - 1.f );
		float eastWeight = MathUtils::SmoothStep( perlinPositionUV.x );
		float northWeight = MathUtils::SmoothStep(perlinPositionUV.y);
		float westWeight = 1.f - eastWeight;
		float southWeight = 1.f - northWeight;

		Vector2 southwestNoiseGradient = GetPseudoRandomNoiseDirection2D( perlinCell.x, perlinCell.y );
		Vector2 southeastNoiseGradient = GetPseudoRandomNoiseDirection2D( perlinCell.x + 1, perlinCell.y );
		Vector2 northeastNoiseGradient = GetPseudoRandomNoiseDirection2D( perlinCell.x + 1, perlinCell.y + 1 );
		Vector2 northwestNoiseGradient = GetPseudoRandomNoiseDirection2D( perlinCell.x, perlinCell.y + 1 );

		float southwestDot = MathUtils::Dot(southwestNoiseGradient, perlinPositionUV);
		float southeastDot = MathUtils::Dot(southeastNoiseGradient, Vector2(perlinPositionAntiUV.x, perlinPositionUV.y));
		float northeastDot = MathUtils::Dot(northeastNoiseGradient, perlinPositionAntiUV);
		float northwestDot = MathUtils::Dot(northwestNoiseGradient, Vector2(perlinPositionUV.x, perlinPositionAntiUV.y));

		float southBlend = (eastWeight * southeastDot) + (westWeight * southwestDot);
		float northBlend = (eastWeight * northeastDot) + (westWeight * northwestDot);
		float fourWayBlend = (southWeight * southBlend) + (northWeight * northBlend);
		float perlinNoiseForThisOctave = currentOctaveAmplitude * fourWayBlend;

		totalPerlinNoise += perlinNoiseForThisOctave;
		perlinGridFrequency *= 2.f;
		totalMaxAmplitude += currentOctaveAmplitude;
		currentOctaveAmplitude *= persistence;
	}

	if( totalMaxAmplitude != 0.f )
		totalPerlinNoise /= totalMaxAmplitude;

	return totalPerlinNoise;
}


