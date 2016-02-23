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
void Block::SetVisible(Direction visibleFace)
{
	uchar bitmaskFaceVisible = 0x00;
	switch (visibleFace)
	{
	case Direction::ABOVE:
		bitmaskFaceVisible = BITMASK_ABOVE_VISIBLE;
		break;
	case Direction::BELOW:
		bitmaskFaceVisible = BITMASK_BELOW_VISIBLE;
		break;
	case Direction::NORTH:
		bitmaskFaceVisible = BITMASK_NORTH_VISIBLE;
		break;
	case Direction::SOUTH:
		bitmaskFaceVisible = BITMASK_SOUTH_VISIBLE;
		break;
	case Direction::EAST:
		bitmaskFaceVisible = BITMASK_EAST_VISIBLE;
		break;
	case Direction::WEST:
		bitmaskFaceVisible = BITMASK_WEST_VISIBLE;
		break;
	default:
		break;
	}
	m_hsrFlags &= ~bitmaskFaceVisible;
	m_hsrFlags |= bitmaskFaceVisible;
}

void Block::SetHidden(Direction hiddenFace)
{
	uchar bitmaskFaceVisible = 0x00;
	switch (hiddenFace)
	{
	case Direction::ABOVE:
		bitmaskFaceVisible = BITMASK_ABOVE_VISIBLE;
		break;
	case Direction::BELOW:
		bitmaskFaceVisible = BITMASK_BELOW_VISIBLE;
		break;
	case Direction::NORTH:
		bitmaskFaceVisible = BITMASK_NORTH_VISIBLE;
		break;
	case Direction::SOUTH:
		bitmaskFaceVisible = BITMASK_SOUTH_VISIBLE;
		break;
	case Direction::EAST:
		bitmaskFaceVisible = BITMASK_EAST_VISIBLE;
		break;
	case Direction::WEST:
		bitmaskFaceVisible = BITMASK_WEST_VISIBLE;
		break;
	default:
		break;
	}
	m_hsrFlags &= ~bitmaskFaceVisible;
}
