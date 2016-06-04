#pragma once
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Math/Vector2Int.hpp"
#include <map>
#include <vector>
#include <string>
#include "../Core/Memory/UntrackedAllocator.hpp"

//---------------------------------------------------------------------------
struct Glyph
{
    Glyph() {};
    Glyph(char character, int x, int y, int width, int height, int xOffset, int yOffset, int xAdvance)
        : id(character)
        , x(x)
        , y(y)
        , width(width)
        , height(height)
        , xOffset(xOffset)
        , yOffset(yOffset)
        , xAdvance(xAdvance) {};

    char id;
    int x;
    int y;
    int width;
    int height;
    int xOffset;
    int yOffset;
    int xAdvance;
};

//---------------------------------------------------------------------------
struct Kerning
{
    Kerning() {};
    Kerning(char first, char second, int amount) : kerningPair(first, second), kerningOffsetAmount(amount) {};
    std::pair<char, char> kerningPair;
    int kerningOffsetAmount;
};

//---------------------------------------------------------------------------
class BitmapFont
{
public:
    //HELPER FUNCTIONS//////////////////////////////////////////////////////////////////////////
    static BitmapFont* GetFontByName(const std::string& imageFilePath);
    static BitmapFont* CreateOrGetFont(const std::string& bitmapFontName);
    static BitmapFont* CreateOrGetFontFromGlyphSheet(const std::string& bitmapFontName);
    static void CleanUpBitmapFontRegistry();
    float CalcTextWidth(const std::string& textToWrite, float scale) const;

    //GETTERS//////////////////////////////////////////////////////////////////////////
    AABB2 GetTexCoordsForGlyph(int glyphAscii) const;
    AABB2 GetTexCoordsForGlyph(const Glyph& glyph) const;
    Texture* GetTexture() const;
    Material* GetMaterial() const;
    const Glyph* GetGlyph(char glyphAscii) const;
    const Vector2 GetKerning(const Glyph& prevGlyph, const Glyph& currentGlyph) const;
    int GetCharacterWidth();

    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    bool m_isMonospaced;
    int m_maxHeight;

private:
    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    BitmapFont(const std::string& bitmapFontName);
    BitmapFont(const std::string& bitmapFontName, const std::string& glyphFileName);
    ~BitmapFont();

    //HELPER FUNCTIONS//////////////////////////////////////////////////////////////////////////
    void ParseGlyphInfo(std::vector<std::string>& glyphSheet);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    static std::map<size_t, BitmapFont*, std::less<size_t>, UntrackedAllocator<std::pair<size_t, BitmapFont*>>> s_fontRegistry;
    static const int CHARACTER_WIDTH = 16;

    SpriteSheet m_spriteSheet;
    Material* m_material;
    Vector2Int m_imageDimensions;
    std::map<char, Glyph> m_glyphMap;
    std::map<std::pair<char, char>, int> m_kerningMap;
};
