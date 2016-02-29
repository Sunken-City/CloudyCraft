#include "Block.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Face.hpp"

//-----------------------------------------------------------------------------------
Block::Block(unsigned char type)
: m_type(type)
, m_lightAndFlags(0)
, m_redLight(0)
, m_greenLight(0)
, m_blueLight(0)
{
}

//-----------------------------------------------------------------------------------
Block::Block()
: m_type(0)
, m_lightAndFlags(0)
, m_redLight(0)
, m_greenLight(0)
, m_blueLight(0)
{
}

//-----------------------------------------------------------------------------------
void Block::Update(float deltaTime)
{
	UNUSED(deltaTime);
}

//-----------------------------------------------------------------------------------
void Block::Render() const
{

}

//-----------------------------------------------------------------------------------
Face Block::GetFace(const Vector3Int& bottomLeftCorner, const Vector3Int& faceNormal)
{
	Vector3 origin = Vector3(bottomLeftCorner);
	Vector3 offset = Vector3(faceNormal) * 0.001f;
	if (faceNormal.y == -1)
	{
		Vector3 mins = origin + offset;
		Vector3 second = mins + Vector3::UNIT_X + offset;
		Vector3 maxs = mins + Vector3(1.0f, 0.0f, 1.0f) + offset;
		Vector3 fourth = mins + Vector3::UNIT_Z + offset;
		return Face(mins, second, maxs, fourth);
	}
	else if (faceNormal.y == 1)
	{
		Vector3 mins = origin + Vector3::UNIT_Y + offset;
		Vector3 second = mins + Vector3::UNIT_Z + offset;
		Vector3 maxs = mins + Vector3(1.0f, 0.0f, 1.0f) + offset;
		Vector3 fourth = mins + Vector3::UNIT_X + offset;
		return Face(mins, second, maxs, fourth);
	}
	else if (faceNormal.x == -1)
	{
		Vector3 mins = origin + offset;
		Vector3 second = mins + Vector3::UNIT_Z + offset;
		Vector3 maxs = mins + Vector3(0.0f, 1.0f, 1.0f) + offset;
		Vector3 fourth = mins + Vector3::UNIT_Y + offset;
		return Face(mins, second, maxs, fourth);
	}
	else if (faceNormal.x == 1)
	{
		Vector3 mins = origin + Vector3::UNIT_X + offset;
		Vector3 second = mins + Vector3::UNIT_Y + offset;
		Vector3 maxs = mins + Vector3(0.0f, 1.0f, 1.0f) + offset;
		Vector3 fourth = mins + Vector3::UNIT_Z + offset;
		return Face(mins, second, maxs, fourth);
	}
	else if (faceNormal.z == -1)
	{
		Vector3 mins = origin + offset;
		Vector3 second = mins + Vector3::UNIT_Y + offset;
		Vector3 maxs = mins + Vector3(1.0f, 1.0f, 0.0f) + offset;
		Vector3 fourth = mins + Vector3::UNIT_X + offset;
		return Face(mins, second, maxs, fourth);
	}
	else if (faceNormal.z == 1)
	{
		Vector3 mins = origin + Vector3::UNIT_Z + offset;
		Vector3 second = mins + Vector3::UNIT_X + offset;
		Vector3 maxs = mins + Vector3(1.0f, 1.0f, 0.0f) + offset;
		Vector3 fourth = mins + Vector3::UNIT_Y + offset;
		return Face(mins, second, maxs, fourth);
	}
	return Face(Vector3::ZERO, Vector3::ZERO, Vector3::ZERO, Vector3::ZERO);
}

//-----------------------------------------------------------------------------------
bool Block::IsPortal(Direction portalFace)
{
	uchar bitmaskPortalFace = 0x00;
	switch (portalFace)
	{
	case Direction::ABOVE:
		return (m_portalFlags & PORTAL_ABOVE_BIT) != 0;
	case Direction::BELOW:
		return (m_portalFlags & PORTAL_BELOW_BIT) != 0;
	case Direction::NORTH:
		return (m_portalFlags & PORTAL_NORTH_BIT) != 0;
	case Direction::SOUTH:
		return (m_portalFlags & PORTAL_SOUTH_BIT) != 0;
	case Direction::EAST:
		return (m_portalFlags & PORTAL_EAST_BIT) != 0;
	case Direction::WEST:
		return (m_portalFlags & PORTAL_WEST_BIT) != 0;
	default:
		return false;
	}
}

//-----------------------------------------------------------------------------------
void Block::SetPortal(Direction portalFace)
{
	uchar bitmaskPortalFace = 0x00;
	switch (portalFace)
	{
	case Direction::ABOVE:
		bitmaskPortalFace = BITMASK_ABOVE_HAS_PORTAL;
		break;
	case Direction::BELOW:
		bitmaskPortalFace = BITMASK_BELOW_HAS_PORTAL;
		break;
	case Direction::NORTH:
		bitmaskPortalFace = BITMASK_NORTH_HAS_PORTAL;
		break;
	case Direction::SOUTH:
		bitmaskPortalFace = BITMASK_SOUTH_HAS_PORTAL;
		break;
	case Direction::EAST:
		bitmaskPortalFace = BITMASK_EAST_HAS_PORTAL;
		break;
	case Direction::WEST:
		bitmaskPortalFace = BITMASK_WEST_HAS_PORTAL;
		break;
	default:
		break;
	}
	m_portalFlags &= ~bitmaskPortalFace;
	m_portalFlags |= bitmaskPortalFace;
}

//-----------------------------------------------------------------------------------
void Block::RemovePortal(Direction hiddenFace)
{
	uchar bitmaskPortalFace = 0x00;
	switch (hiddenFace)
	{
	case Direction::ABOVE:
		bitmaskPortalFace = BITMASK_ABOVE_HAS_PORTAL;
		break;
	case Direction::BELOW:
		bitmaskPortalFace = BITMASK_BELOW_HAS_PORTAL;
		break;
	case Direction::NORTH:
		bitmaskPortalFace = BITMASK_NORTH_HAS_PORTAL;
		break;
	case Direction::SOUTH:
		bitmaskPortalFace = BITMASK_SOUTH_HAS_PORTAL;
		break;
	case Direction::EAST:
		bitmaskPortalFace = BITMASK_EAST_HAS_PORTAL;
		break;
	case Direction::WEST:
		bitmaskPortalFace = BITMASK_WEST_HAS_PORTAL;
		break;
	default:
		break;
	}
	m_portalFlags &= ~bitmaskPortalFace;
}
