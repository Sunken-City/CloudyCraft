#include "Engine/Renderer/RGBA.hpp"

const RGBA RGBA::WHITE(0xFFFFFFFF);
const RGBA RGBA::BLACK(0x000000FF);
const RGBA RGBA::RED(0xFF0000FF);
const RGBA RGBA::GREEN(0x00FF00FF);
const RGBA RGBA::BLUE(0x0000FFFF);
const RGBA RGBA::ORANGE(0xFFBB00FF);
const RGBA RGBA::MAGENTA(0xFF00FFFF);
const RGBA RGBA::YELLOW(0xFFFF00FF);
const RGBA RGBA::CYAN(0x00FFFFFF);

RGBA::RGBA() : red(0x00), green(0x00), blue(0x00), alpha(0xFF)
{
}

RGBA::RGBA(float Red, float Green, float Blue) 
: red(static_cast<unsigned char>((255.0f * Red)))
, green(static_cast<unsigned char>((255.0f * Green)))
, blue(static_cast<unsigned char>((255.0f * Blue)))
, alpha(0xFF)
{

}

RGBA::RGBA(float Red, float Green, float Blue, float Alpha)
: red(static_cast<unsigned char>((255.0f * Red)))
, green(static_cast<unsigned char>((255.0f * Green)))
, blue(static_cast<unsigned char>((255.0f * Blue)))
, alpha(static_cast<unsigned char>((255.0f * Alpha)))
{

}

RGBA::RGBA(unsigned int hexColor)
: red(GetRed(hexColor))
, green(GetGreen(hexColor))
, blue(GetBlue(hexColor))
, alpha((unsigned char)((hexColor & ALPHA_MASK)))
{
}

RGBA::~RGBA()
{
}