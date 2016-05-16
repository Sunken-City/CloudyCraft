#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Input/InputOutputUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

std::map< std::string, BitmapFont* > BitmapFont::s_fontRegistry;

//-----------------------------------------------------------------------------------
AABB2 BitmapFont::GetTexCoordsForGlyph(int glyphAscii) const
{
    return m_spriteSheet.GetTexCoordsForSpriteIndex(glyphAscii);
}

//-----------------------------------------------------------------------------------
AABB2 BitmapFont::GetTexCoordsForGlyph(const Glyph& glyph) const
{
    const Vector2 bottomLeft = Vector2((float)glyph.x / (float)m_imageDimensions.x, (float)glyph.y / (float)m_imageDimensions.y);
    const Vector2 topRight = bottomLeft + Vector2((float)glyph.width / (float)m_imageDimensions.x, (float)glyph.height / (float)m_imageDimensions.x);

    //Because STBI is terrible and flips our images, we need to correct how textures are rendered.
    //Instead of (0,0) (1,1), we need to get the coordinates in (0,1) (1,0) order. This renders upright and correctly.
    //TODO: IF STBI GETS FIXED OR REPLACED, YOU'LL NEED TO REMOVE THIS AND MAKE IT MAKE MORE SENSE.
    AABB2 texCoords;
    texCoords.maxs.x = topRight.x;
    texCoords.maxs.y = bottomLeft.y;
    texCoords.mins.x = bottomLeft.x;
    texCoords.mins.y = topRight.y;
    return texCoords;
}

//-----------------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------------
BitmapFont* BitmapFont::CreateOrGetFont(const std::string& bitmapFontName)
{
    BitmapFont* font = GetFontByName(bitmapFontName);
    if (font != nullptr)
    {
        return font;
    }
    else
    {
        if (FileExists("Data/Fonts/" + bitmapFontName + ".png"))
        {
            font = new BitmapFont(bitmapFontName);
            BitmapFont::s_fontRegistry[bitmapFontName] = font;
            return font;
        }
        else
        {
            return nullptr;
        }
    }
}

//-----------------------------------------------------------------------------------
BitmapFont* BitmapFont::CreateOrGetFontFromGlyphSheet(const std::string& bitmapFontName)
{
    BitmapFont* font = GetFontByName(bitmapFontName);
    if (font != nullptr)
    {
        return font;
    }
    else
    {
        if (FileExists("Data/Fonts/" + bitmapFontName + "_0.png"))
        {
            font = new BitmapFont(bitmapFontName + "_0", bitmapFontName);
            BitmapFont::s_fontRegistry[bitmapFontName] = font;
            return font;
        }
        else
        {
            return nullptr;
        }
    }
}

//-----------------------------------------------------------------------------------
Texture* BitmapFont::GetTexture() const
{
    return m_spriteSheet.GetTexture();
}

Material* BitmapFont::GetMaterial() const
{
    return m_material;
}

//-----------------------------------------------------------------------------------
const Glyph* BitmapFont::GetGlyph(char glyphAscii) const
{
    auto characterIterator = m_glyphMap.find(glyphAscii);
    if (characterIterator == BitmapFont::m_glyphMap.end())
    {
        DebuggerPrintf("Font doesn't have a value for character '%c'. Using space instead.", glyphAscii);
        return &m_glyphMap.find(' ')->second;
    }
    return &characterIterator->second;
}

//-----------------------------------------------------------------------------------
int BitmapFont::GetCharacterWidth()
{
    return CHARACTER_WIDTH;
}

//-----------------------------------------------------------------------------------
float BitmapFont::CalcTextWidth(const std::string& textToWrite, float scale) const
{
    const Glyph* previousGlyph = nullptr;
    float totalWidth = 0.0f;
    for (char character : textToWrite)
    {
        const Glyph* glyph = GetGlyph(character);
        if (previousGlyph)
        {
            const Vector2 kerning = GetKerning(*previousGlyph, *glyph);
            totalWidth += (kerning.x * scale);
        }
        totalWidth += glyph->xAdvance;
        previousGlyph = glyph;
    }
    return totalWidth;
}

//-----------------------------------------------------------------------------------
BitmapFont::BitmapFont(const std::string& bitmapFontName)
    : m_spriteSheet("Data/Fonts/" + bitmapFontName + ".png", CHARACTER_WIDTH, CHARACTER_WIDTH)
    , m_isMonospaced(true)
    , m_maxHeight(CHARACTER_WIDTH)
    , m_material(new Material(new ShaderProgram("Data/Shaders/fixedVertexFormat.vert", "Data/Shaders/fixedVertexFormat.frag"),
        RenderState(RenderState::DepthTestingMode::OFF, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)))
{
    m_imageDimensions = m_spriteSheet.GetTexture()->m_texelSize;
    m_material->SetDiffuseTexture(m_spriteSheet.GetTexture());
}

//-----------------------------------------------------------------------------------
BitmapFont::BitmapFont(const std::string& bitmapFontName, const std::string& glyphFileName)
    : m_spriteSheet("Data/Fonts/" + bitmapFontName + ".png", CHARACTER_WIDTH, CHARACTER_WIDTH)
    , m_isMonospaced(false)
    , m_maxHeight(0)
    , m_material(new Material(new ShaderProgram("Data/Shaders/fixedVertexFormat.vert", "Data/Shaders/fixedVertexFormat.frag"),
        RenderState(RenderState::DepthTestingMode::OFF, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)))
{
    m_imageDimensions = m_spriteSheet.GetTexture()->m_texelSize;
    std::vector<std::string> glyphSheet;
    ReadTextFileIntoVector(glyphSheet, "Data/Fonts/" + glyphFileName + ".fnt");
    ParseGlyphInfo(glyphSheet);
    m_material->SetDiffuseTexture(m_spriteSheet.GetTexture());
}

//-----------------------------------------------------------------------------------
void BitmapFont::ParseGlyphInfo(std::vector<std::string>& glyphMetaFile)
{
    const int ID_INDEX = 0;
    const int X_INDEX = 1;
    const int Y_INDEX = 2;
    const int WIDTH_INDEX = 3;
    const int HEIGHT_INDEX = 4;
    const int X_OFFSET_INDEX = 5;
    const int Y_OFFSET_INDEX = 6;
    const int X_ADVANCE_INDEX = 7;
    const int FIRST_INDEX = 0;
    const int SECOND_INDEX = 1;
    const int AMOUNT_INDEX = 2;
    std::vector<std::string>* values;
    int numLines = glyphMetaFile.size();
    //Find the number of characters
    int lineIndex = 3;
    values = ExtractStringsBetween(glyphMetaFile[lineIndex++], "=", "\r");
    int numberOfCharacters = std::stoi((*values)[0]);
    delete values;
    for (int i = 0; i < numberOfCharacters; i++)
    {
        Glyph letter;
        std::string& currentString = glyphMetaFile[lineIndex + i];
        values = ExtractStringsBetween(currentString, "=", " ");
        letter.id = (char)std::stoi((*values)[ID_INDEX]);
        letter.x = std::stoi((*values)[X_INDEX]);
        letter.y = std::stoi((*values)[Y_INDEX]);
        letter.width = std::stoi((*values)[WIDTH_INDEX]);
        letter.height = std::stoi((*values)[HEIGHT_INDEX]);
        letter.xOffset = std::stoi((*values)[X_OFFSET_INDEX]);
        letter.yOffset = std::stoi((*values)[Y_OFFSET_INDEX]);
        letter.xAdvance = std::stoi((*values)[X_ADVANCE_INDEX]);

        m_glyphMap.emplace((char)letter.id, letter);
        int letterHeight = letter.yOffset + letter.height;
        m_maxHeight = letterHeight > m_maxHeight ? letterHeight : m_maxHeight;
        delete values;
    }
    lineIndex += numberOfCharacters;
    //If we're not at the end of file, do the kerning info
    if (numLines == lineIndex)
    {
        return;
    }
    values = ExtractStringsBetween(glyphMetaFile[lineIndex++], "=", "\r");
    int numberOfKernings = std::stoi((*values)[0]);
    delete values;
    for (int i = 0; i < numberOfKernings; i++)
    {
        std::string& currentString = glyphMetaFile[lineIndex + i];
        values = ExtractStringsBetween(currentString, "=", " ");
        char first = (char)std::stoi((*values)[FIRST_INDEX]);
        char second = (char)std::stoi((*values)[SECOND_INDEX]);
        int amount = std::stoi((*values)[AMOUNT_INDEX]);
        Kerning info = Kerning(first, second, amount);
        m_kerningMap.emplace(info.kerningPair, info.kerningOffsetAmount);
        delete values;
    }
}

//-----------------------------------------------------------------------------------
const Vector2 BitmapFont::GetKerning(const Glyph& prevGlyph, const Glyph& currentGlyph) const
{
    auto characterIterator = m_kerningMap.find(std::pair<char, char>(prevGlyph.id, currentGlyph.id));
    if (characterIterator == BitmapFont::m_kerningMap.end())
    {
        return Vector2::ZERO;
    }
    return Vector2(static_cast<float>(characterIterator->second), 0.0f);
}
