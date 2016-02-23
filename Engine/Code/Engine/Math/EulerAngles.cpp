#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <cmath>

//-----------------------------------------------------------------------------------
EulerAngles::EulerAngles()
{
}

//-----------------------------------------------------------------------------------
EulerAngles::EulerAngles(float initialX, float initialY, float initialZ)
	: rollDegreesAboutX(initialX)
	, pitchDegreesAboutY(initialY)
	, yawDegreesAboutZ(initialZ)
{
}

//-----------------------------------------------------------------------------------
EulerAngles::EulerAngles(const EulerAngles& other) 
	: rollDegreesAboutX(other.rollDegreesAboutX)
	, pitchDegreesAboutY(other.pitchDegreesAboutY)
	, yawDegreesAboutZ(other.yawDegreesAboutZ)
{
}

//-----------------------------------------------------------------------------------
void EulerAngles::SetXYZ(float newX, float newY, float newZ)
{
	rollDegreesAboutX = newX;
	pitchDegreesAboutY = newY;
	yawDegreesAboutZ = newZ;
}

//-----------------------------------------------------------------------------------
float EulerAngles::CalculateMagnitude()
{
	return sqrt((rollDegreesAboutX*rollDegreesAboutX) + (pitchDegreesAboutY*pitchDegreesAboutY) + (yawDegreesAboutZ*yawDegreesAboutZ));
}

//-----------------------------------------------------------------------------------
void EulerAngles::Normalize()
{
	float len = CalculateMagnitude();
	if (len == 0.f)return;
	rollDegreesAboutX /= len;
	pitchDegreesAboutY /= len;
	yawDegreesAboutZ /= len;
}

//-----------------------------------------------------------------------------------
EulerAngles& EulerAngles::operator+=(const EulerAngles& rhs)
{
	this->rollDegreesAboutX += rhs.rollDegreesAboutX;
	this->pitchDegreesAboutY += rhs.pitchDegreesAboutY;
	this->yawDegreesAboutZ += rhs.yawDegreesAboutZ;
	return *this;
}

//-----------------------------------------------------------------------------------
EulerAngles& EulerAngles::operator-=(const EulerAngles& rhs)
{
	this->rollDegreesAboutX -= rhs.rollDegreesAboutX;
	this->pitchDegreesAboutY -= rhs.pitchDegreesAboutY;
	this->yawDegreesAboutZ -= rhs.yawDegreesAboutZ;
	return *this;
}

//-----------------------------------------------------------------------------------
EulerAngles& EulerAngles::operator*=(const float& scalarConstant)
{
	this->rollDegreesAboutX *= scalarConstant;
	this->pitchDegreesAboutY *= scalarConstant;
	this->yawDegreesAboutZ *= scalarConstant;
	return *this;
}
