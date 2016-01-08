#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/AABB2.hpp"

std::map< std::string, BitmapFont* > BitmapFont::s_fontRegistry;

AABB2 BitmapFont::GetTexCoordsForGlyph(int glyphAscii) const
{
	return m_spriteSheet.GetTexCoordsForSpriteIndex(glyphAscii);
}

BitmapFont* BitmapFont::GetFontByName(const std::string& imageFilePath)
{
	auto iterator = BitmapFont::s_fontRegistry.find(imageFilePath);
	if (iterator == BitmapFont::s_fontRegistry.end())
	{
		return nullptr;
	}
	else
	{
		return iterator->second;
	}
}


BitmapFont* BitmapFont::CreateOrGetFont(const std::string& bitmapFontName)
{
	BitmapFont* font = GetFontByName(bitmapFontName);
	if (font != nullptr)
	{
		return font;
	}
	else
	{
		font = new BitmapFont(bitmapFontName);
		BitmapFont::s_fontRegistry[bitmapFontName] = font;
		return font;
	}
}

Texture* BitmapFont::GetTexture() const
{
	return m_spriteSheet.GetTexture();
}

int BitmapFont::GetCharacterWidth()
{
	return CHARACTER_WIDTH;
}

BitmapFont::BitmapFont(const std::string& bitmapFontName)
: m_spriteSheet("Data/Fonts/" + bitmapFontName + ".png", CHARACTER_WIDTH, CHARACTER_WIDTH)
{

}
