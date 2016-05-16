#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/EulerAngles.hpp"

class Player;
class Matrix4x4;

class Camera3D
{
public:
	//ENUMS//////////////////////////////////////////////////////////////////////////
	enum class CameraMode
	{
		FIRST_PERSON = 0,
		FROM_BEHIND,
		FIXED_ANGLE_TRACKING,
		NUM_CAMERA_MODES
	};

	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
	Camera3D(Player* player);

	//FUNCTIONS//////////////////////////////////////////////////////////////////////////
	void FixAndClampAngles(); //Prevents pitch from going above 89.9
	Vector3 GetForwardXYZ() const;
	Vector3 GetForwardXY() const;
	Vector3 GetLeftXY() const;
	void UpdateViewFromCamera() const;
	void ExitViewFromCamera();

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	mutable Vector3 m_position;
	mutable EulerAngles m_orientation;
	CameraMode m_cameraMode;
	Player* m_player;
};
