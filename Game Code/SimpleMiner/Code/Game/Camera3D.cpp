#include "Game/Camera3D.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/TheRenderer.hpp"
#include "Game/Player.hpp"
#include "Game/World.hpp"

Camera3D::Camera3D(Player* player)
: m_orientation(0.f, 0.f, 0.f)
, m_position(8.f, 8.f, 100.f)
, m_cameraMode(CameraMode::FIRST_PERSON)
, m_player(player)
{
}

Vector3 Camera3D::GetForwardXYZ() const
{
	float cosYaw = MathUtils::CosDegrees(m_orientation.yawDegreesAboutZ);
	float sinYaw = MathUtils::SinDegrees(m_orientation.yawDegreesAboutZ);
	float cosPitch = MathUtils::CosDegrees(m_orientation.pitchDegreesAboutY);
	float sinPitch = MathUtils::SinDegrees(m_orientation.pitchDegreesAboutY);
	return Vector3(cosYaw * cosPitch, sinYaw * cosPitch, -sinPitch);
}

Vector3 Camera3D::GetForwardXY() const
{
	float cosYaw = MathUtils::CosDegrees(m_orientation.yawDegreesAboutZ);
	float sinYaw = MathUtils::SinDegrees(m_orientation.yawDegreesAboutZ);
	return Vector3(cosYaw, sinYaw, 0.f);
}

Vector3 Camera3D::GetLeftXY() const
{
	Vector3 forwardXY = GetForwardXY();
	return Vector3(-forwardXY.y, forwardXY.x, 0.f);
}

void Camera3D::UpdateViewFromCamera() const
{
	if (m_cameraMode == CameraMode::FIXED_ANGLE_TRACKING)
	{
		TheRenderer::instance->Rotate(-20.0f, 0.f, 1.f, 0.f);
		TheRenderer::instance->Rotate(-45.0f, 0.f, 0.f, 1.f);
		TheRenderer::instance->Rotate(-0.0f, 1.f, 0.f, 0.f);
		Vector3 offset = Vector3(-9.0f, -6.0f, 5.0f);
		RaycastResult3D raycast = m_player->m_world->Raycast(m_position, m_position + offset);
		if (raycast.didImpact)
		{
			offset *= raycast.impactFraction;
		}
		TheRenderer::instance->Translate(-(m_position.x + offset.x), -(m_position.y + offset.y), -(m_position.z + offset.z));
		return;
	}
	TheRenderer::instance->Rotate(-m_orientation.pitchDegreesAboutY, 0.f, 1.f, 0.f);
	TheRenderer::instance->Rotate(-m_orientation.yawDegreesAboutZ, 0.f, 0.f, 1.f);
	TheRenderer::instance->Rotate(-m_orientation.rollDegreesAboutX, 1.f, 0.f, 0.f);
	TheRenderer::instance->Translate(-m_position.x, -m_position.y, -m_position.z);
	if (m_cameraMode == CameraMode::FROM_BEHIND)
	{
		Vector3 offset = GetForwardXYZ() * -4.0f;
		RaycastResult3D raycast = m_player->m_world->Raycast(m_position, m_position + offset);
		if (raycast.didImpact)
		{
			offset *= raycast.impactFraction;
		}
		TheRenderer::instance->Translate(-offset.x, -offset.y, -offset.z);
	}
}
