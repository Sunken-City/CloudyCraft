#pragma once

#include "Engine/Renderer/SpriteSheet.hpp"
#include <map>
#include <string>

class BitmapFont
{
public:
	static BitmapFont* GetFontByName(const std::string& imageFilePath);
	static BitmapFont* CreateOrGetFont(const std::string& bitmapFontName);
	AABB2 GetTexCoordsForGlyph(int glyphAscii) const;
	Texture* GetTexture() const;
	int GetCharacterWidth();

private:
	BitmapFont(const std::string& bitmapFontName);
	static const int CHARACTER_WIDTH = 16;

	SpriteSheet m_spriteSheet;
	static std::map< std::string, BitmapFont* > s_fontRegistry;
};
