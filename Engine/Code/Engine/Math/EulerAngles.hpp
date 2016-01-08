#pragma once

class EulerAngles
{
public:
	EulerAngles();
	EulerAngles(float initialX, float initialY, float initialZ);
	EulerAngles(const EulerAngles& other);
	void SetXYZ(float newX, float newY, float newZ);
	float CalculateMagnitude();
	void Normalize();

	EulerAngles& operator+=(const EulerAngles& rhs);
	EulerAngles& operator-=(const EulerAngles& rhs);
	EulerAngles& operator*=(const float& scalarConstant);
private:

public:
	float rollDegreesAboutX;
	float pitchDegreesAboutY;
	float yawDegreesAboutZ;
};

inline EulerAngles operator+(EulerAngles lhs, const EulerAngles& rhs)
{
	lhs += rhs;
	return lhs;
}

inline EulerAngles operator-(EulerAngles lhs, const EulerAngles& rhs)
{
	lhs -= rhs;
	return lhs;
}

inline EulerAngles operator*(EulerAngles lhs, const float& scalarConstant)
{
	lhs *= scalarConstant;
	return lhs;
}

inline bool operator==(const EulerAngles& lhs, const EulerAngles& rhs)
{ 
	return (lhs.rollDegreesAboutX == rhs.rollDegreesAboutX) && (lhs.pitchDegreesAboutY == rhs.pitchDegreesAboutY) && (lhs.yawDegreesAboutZ == rhs.yawDegreesAboutZ); 
}

inline bool operator!=(const EulerAngles& lhs, const EulerAngles& rhs)
{
	return !(lhs == rhs);
}