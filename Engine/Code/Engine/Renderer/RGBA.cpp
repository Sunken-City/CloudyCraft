#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/MathUtils.hpp"

const RGBA RGBA::WHITE(0xFFFFFFFF);
const RGBA RGBA::BLACK(0x000000FF);
const RGBA RGBA::RED(0xFF0000FF);
const RGBA RGBA::GREEN(0x00FF00FF);
const RGBA RGBA::BLUE(0x0000FFFF);
const RGBA RGBA::MAGENTA(0xFF00FFFF);
const RGBA RGBA::YELLOW(0xFFFF00FF);
const RGBA RGBA::CYAN(0x00FFFFFF);
const RGBA RGBA::ORANGE(0xFFBB00FF);
const RGBA RGBA::CORNFLOWER_BLUE(0x6495EDFF);
const RGBA RGBA::TURQUOISE(0x00868BFF);
const RGBA RGBA::SLATE(0x2F4F4FFF);
const RGBA RGBA::FOREST_GREEN(0x228B22FF);
const RGBA RGBA::SEA_GREEN(0x43CD80FF);
const RGBA RGBA::KHAKI(0xCDC673FF);
const RGBA RGBA::GOLD(0xCDAD00FF);
const RGBA RGBA::CHOCOLATE(0xD2691EFF);
const RGBA RGBA::SADDLE_BROWN(0x8B4513FF);
const RGBA RGBA::MAROON(0x800000FF);
const RGBA RGBA::GRAY(0x808080FF);
const RGBA RGBA::VAPORWAVE(0xFF819CFF);

//-----------------------------------------------------------------------------------
RGBA::RGBA() : red(0x00), green(0x00), blue(0x00), alpha(0xFF)
{
}

//-----------------------------------------------------------------------------------
RGBA::RGBA(float Red, float Green, float Blue) 
	: red(static_cast<unsigned char>((255.0f * Red)))
	, green(static_cast<unsigned char>((255.0f * Green)))
	, blue(static_cast<unsigned char>((255.0f * Blue)))
	, alpha(0xFF)
{

}

//-----------------------------------------------------------------------------------
RGBA::RGBA(float Red, float Green, float Blue, float Alpha)
	: red(static_cast<unsigned char>((255.0f * Red)))
	, green(static_cast<unsigned char>((255.0f * Green)))
	, blue(static_cast<unsigned char>((255.0f * Blue)))
	, alpha(static_cast<unsigned char>((255.0f * Alpha)))
{

}

//-----------------------------------------------------------------------------------
RGBA::RGBA(unsigned int hexColor)
	: red(GetRed(hexColor))
	, green(GetGreen(hexColor))
	, blue(GetBlue(hexColor))
	, alpha(static_cast<unsigned char>((hexColor & ALPHA_MASK)))
{
}

RGBA::RGBA(const Vector4& color)
	: red(static_cast<unsigned char>((255.0f * color.x)))
	, green(static_cast<unsigned char>((255.0f * color.y)))
	, blue(static_cast<unsigned char>((255.0f * color.z)))
	, alpha(static_cast<unsigned char>((255.0f * color.w)))
{

}

//-----------------------------------------------------------------------------------
RGBA::~RGBA()
{
}

//-----------------------------------------------------------------------------------
Vector4 RGBA::ToVec4() const
{
	return Vector4(static_cast<float>(red) / 255.0f, static_cast<float>(green) / 255.0f, static_cast<float>(blue) / 255.0f, static_cast<float>(alpha) / 255.0f);
}

Vector4 RGBA::RandomVec4()
{
	return Vector4(MathUtils::GetRandomFromZeroTo(1.0f), MathUtils::GetRandomFromZeroTo(1.0f), MathUtils::GetRandomFromZeroTo(1.0f), 1.0f);
}
