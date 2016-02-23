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
inline void Block::SetBelowVisible(bool isVisible)
{
	m_hsrFlags &= ~BITMASK_BELOW_VISIBLE;
	m_hsrFlags |= isVisible ? BITMASK_BELOW_VISIBLE : 0x00;
}

//-----------------------------------------------------------------------------------
inline void Block::SetAboveVisible(bool isVisible)
{
	m_hsrFlags &= ~BITMASK_ABOVE_VISIBLE;
	m_hsrFlags |= isVisible ? BITMASK_ABOVE_VISIBLE : 0x00;
}

//-----------------------------------------------------------------------------------
inline void Block::SetNorthVisible(bool isVisible)
{
	m_hsrFlags &= ~BITMASK_NORTH_VISIBLE;
	m_hsrFlags |= isVisible ? BITMASK_NORTH_VISIBLE : 0x00;
}

//-----------------------------------------------------------------------------------
inline void Block::SetSouthVisible(bool isVisible)
{
	m_hsrFlags &= ~BITMASK_SOUTH_VISIBLE;
	m_hsrFlags |= isVisible ? BITMASK_SOUTH_VISIBLE : 0x00;
}

//-----------------------------------------------------------------------------------
inline void Block::SetEastVisible(bool isVisible)
{
	m_hsrFlags &= ~BITMASK_EAST_VISIBLE;
	m_hsrFlags |= isVisible ? BITMASK_EAST_VISIBLE : 0x00;
}

//-----------------------------------------------------------------------------------
inline void Block::SetWestVisible(bool isVisible)
{
	m_hsrFlags &= ~BITMASK_WEST_VISIBLE;
	m_hsrFlags |= isVisible ? BITMASK_WEST_VISIBLE : 0x00;
}