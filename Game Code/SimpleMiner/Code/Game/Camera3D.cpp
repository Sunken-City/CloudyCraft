#include "Game/Camera3D.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Player.hpp"
#include "Game/World.hpp"

//-----------------------------------------------------------------------------------
Camera3D::Camera3D(Player* player)
: m_orientation(0.f, 0.f, 0.f)
, m_position(8.f, 8.f, 100.f)
, m_cameraMode(CameraMode::FIRST_PERSON)
, m_player(player)
{
}

//-----------------------------------------------------------------------------------
Vector3 Camera3D::GetForwardXYZ() const
{
	float cosYaw = MathUtils::CosDegrees(m_orientation.yawDegreesAboutZ);
	float sinYaw = MathUtils::SinDegrees(m_orientation.yawDegreesAboutZ);
	float cosPitch = MathUtils::CosDegrees(m_orientation.pitchDegreesAboutY);
	float sinPitch = MathUtils::SinDegrees(m_orientation.pitchDegreesAboutY);
	return Vector3(-cosYaw * cosPitch, -sinYaw * cosPitch, sinPitch);
}

//-----------------------------------------------------------------------------------
Vector3 Camera3D::GetForwardXY() const
{
	float cosYaw = MathUtils::CosDegrees(m_orientation.yawDegreesAboutZ);
	float sinYaw = MathUtils::SinDegrees(m_orientation.yawDegreesAboutZ);
	return Vector3(-cosYaw, -sinYaw, 0.f);
}

//-----------------------------------------------------------------------------------
Vector3 Camera3D::GetLeftXY() const
{
	Vector3 forwardXY = GetForwardXY();
	return Vector3(forwardXY.y, -forwardXY.x, 0.f);
}

//-----------------------------------------------------------------------------------
void Camera3D::UpdateViewFromCamera() const
{
	InputSystem::instance->CaptureMouseCursor();
	Vector2Int cursorDelta = InputSystem::instance->GetDeltaMouse();

	m_position = m_player->m_position + Player::EYE_OFFSET;
	m_orientation.yawDegreesAboutZ += ((float)cursorDelta.x * 0.022f);
	float proposedPitch = m_orientation.pitchDegreesAboutY - ((float)cursorDelta.y * 0.022f);
	m_orientation.pitchDegreesAboutY = MathUtils::Clamp(proposedPitch, -89.9f, 89.9f);

	if (m_cameraMode == CameraMode::FIXED_ANGLE_TRACKING)
	{
		Renderer::instance->RotateView(-20.0f, Vector3::UNIT_Y);
		Renderer::instance->RotateView(-45.0f, Vector3::UNIT_Z);
		Renderer::instance->RotateView(-0.0f, Vector3::UNIT_X);
		Vector3 offset = Vector3(-9.0f, -6.0f, 5.0f);
		RaycastResult3D raycast = m_player->m_world->Raycast(m_position, m_position + offset);
		if (raycast.didImpact)
		{
			offset *= raycast.impactFraction;
		}
		Renderer::instance->TranslateView(Vector3(-(m_position.x + offset.x), -(m_position.y + offset.y), -(m_position.z + offset.z)));
		return;
	}
	Renderer::instance->RotateView(-m_orientation.pitchDegreesAboutY, Vector3::UNIT_Y);
	Renderer::instance->RotateView(-m_orientation.yawDegreesAboutZ, Vector3::UNIT_Z);
	Renderer::instance->RotateView(-m_orientation.rollDegreesAboutX, Vector3::UNIT_X);
	Renderer::instance->TranslateView(Vector3(-m_position.x, -m_position.y, -m_position.z));
	if (m_cameraMode == CameraMode::FROM_BEHIND)
	{
		Vector3 offset = GetForwardXYZ() * -4.0f;
		RaycastResult3D raycast = m_player->m_world->Raycast(m_position, m_position + offset);
		if (raycast.didImpact)
		{
			offset *= raycast.impactFraction;
		}
		Renderer::instance->TranslateView(Vector3(-offset.x, -offset.y, -offset.z));
	}
}

//-----------------------------------------------------------------------------------
void Camera3D::ExitViewFromCamera()
{
	if (m_cameraMode == CameraMode::FROM_BEHIND)
	{
		//Pop the extra Translation
		Renderer::instance->PopView();
	}
	//Pop Translation
	Renderer::instance->PopView();
	//Pop rotation about X
	Renderer::instance->PopView();
	//Pop rotation about Z
	Renderer::instance->PopView();
	//Pop rotation about Y
	Renderer::instance->PopView();
}
