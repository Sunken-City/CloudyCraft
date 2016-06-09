#include "Game\Block.hpp"

//-----------------------------------------------------------------------------------
inline unsigned int Block::GetDampedLightValue(uchar dampAmount) const
{
	uchar red = m_redLight - dampAmount;
	uchar green = m_greenLight - dampAmount;
	uchar blue = m_blueLight - dampAmount;
	if (red > m_redLight)
	{
		red = 0x00;
	}
	if (green > m_greenLight)
	{
		green = 0x00;
	}
	if (blue > m_blueLight)
	{
		blue = 0x00;
	}
	return (red << RGBA::SHIFT_RED) + (green << RGBA::SHIFT_GREEN) + (blue << RGBA::SHIFT_BLUE) + 0xFF;
}

//-----------------------------------------------------------------------------------
inline unsigned int Block::GetLightValue() const
{
	return (m_redLight << RGBA::SHIFT_RED) + (m_greenLight << RGBA::SHIFT_GREEN) + (m_blueLight << RGBA::SHIFT_BLUE) + 0xFF;
}

//-----------------------------------------------------------------------------------
inline RGBA Block::GetRGBALightValue() const
{
	return RGBA((m_redLight << RGBA::SHIFT_RED) + (m_greenLight << RGBA::SHIFT_GREEN) + (m_blueLight << RGBA::SHIFT_BLUE) + 0xFF);
}

//-----------------------------------------------------------------------------------
inline uchar Block::GetRedLightValue() const
{
	return m_redLight;
}

//-----------------------------------------------------------------------------------
inline uchar Block::GetGreenLightValue() const
{
	return m_greenLight;
}

//-----------------------------------------------------------------------------------
inline uchar Block::GetBlueLightValue() const
{
	return m_blueLight;
}

//-----------------------------------------------------------------------------------
inline void Block::SetRedLightValue(uchar lightValue0To255)
{
	m_redLight = lightValue0To255;
}

//-----------------------------------------------------------------------------------
inline void Block::SetGreenLightValue(uchar lightValue0To255)
{
	m_greenLight = lightValue0To255;
}

//-----------------------------------------------------------------------------------
inline void Block::SetBlueLightValue(uchar lightValue0To255)
{
	m_blueLight = lightValue0To255;
}

//-----------------------------------------------------------------------------------
inline void Block::SetLightValue(const RGBA& lightColor)
{
	m_redLight = lightColor.red;
	m_greenLight = lightColor.green;
	m_blueLight = lightColor.blue;
}

//-----------------------------------------------------------------------------------
inline void Block::SetEdgeBlock(bool isEdgeBlock)
{
	m_lightAndFlags &= ~BITMASK_BLOCK_IS_EDGE;
	m_lightAndFlags |= isEdgeBlock ? BITMASK_BLOCK_IS_EDGE : 0x00;
}

//-----------------------------------------------------------------------------------
inline void Block::SetSky(bool isSkyBlock)
{
	m_lightAndFlags &= ~BITMASK_BLOCK_IS_SKY;
	m_lightAndFlags |= isSkyBlock ? BITMASK_BLOCK_IS_SKY : 0x00;
}

//-----------------------------------------------------------------------------------
inline void Block::SetDirty(bool isLightingDirty)
{
	m_lightAndFlags &= ~BITMASK_BLOCK_LIGHT_DIRTY;
	m_lightAndFlags |= isLightingDirty ? BITMASK_BLOCK_LIGHT_DIRTY : 0x00;
}

//-----------------------------------------------------------------------------------
inline void Block::SetBelowHasPortal(bool hasPortal)
{
	m_portalFlags &= ~BITMASK_BELOW_HAS_PORTAL;
	m_portalFlags |= hasPortal ? BITMASK_BELOW_HAS_PORTAL : 0x00;
}

//-----------------------------------------------------------------------------------
inline void Block::SetAboveHasPortal(bool hasPortal)
{
	m_portalFlags &= ~BITMASK_ABOVE_HAS_PORTAL;
	m_portalFlags |= hasPortal ? BITMASK_ABOVE_HAS_PORTAL : 0x00;
}

//-----------------------------------------------------------------------------------
inline void Block::SetNorthHasPortal(bool hasPortal)
{
	m_portalFlags &= ~BITMASK_NORTH_HAS_PORTAL;
	m_portalFlags |= hasPortal ? BITMASK_NORTH_HAS_PORTAL : 0x00;
}

//-----------------------------------------------------------------------------------
inline void Block::SetSouthHasPortal(bool hasPortal)
{
	m_portalFlags &= ~BITMASK_SOUTH_HAS_PORTAL;
	m_portalFlags |= hasPortal ? BITMASK_SOUTH_HAS_PORTAL : 0x00;
}

//-----------------------------------------------------------------------------------
inline void Block::SetEastHasPortal(bool isVisible)
{
	m_portalFlags &= ~BITMASK_EAST_HAS_PORTAL;
	m_portalFlags |= isVisible ? BITMASK_EAST_HAS_PORTAL : 0x00;
}

//-----------------------------------------------------------------------------------
inline void Block::SetWestHasPortal(bool isVisible)
{
	m_portalFlags &= ~BITMASK_WEST_HAS_PORTAL;
	m_portalFlags |= isVisible ? BITMASK_WEST_HAS_PORTAL : 0x00;
}