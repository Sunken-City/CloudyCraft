#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Renderer/Texture.hpp"

//-----------------------------------------------------------------------------------
SpriteSheet::SpriteSheet(const std::string& imageFilePath, int tilesWide, int tilesHigh) 
: m_spriteLayout(Vector2Int(tilesWide, tilesHigh))
, m_spriteSheetTexture(Texture::CreateOrGetTexture(imageFilePath))
, m_texCoordsPerTile(Vector2(1.0f / tilesWide, 1.0f / tilesHigh))
{

}

//-----------------------------------------------------------------------------------
AABB2 SpriteSheet::GetTexCoordsForSpriteCoords(const Vector2Int& spriteCoords) const
{
	AABB2 texCoords;
	texCoords.mins.x = m_texCoordsPerTile.x * static_cast<float>(spriteCoords.x);
	texCoords.mins.y = m_texCoordsPerTile.y * static_cast<float>(spriteCoords.y);

	texCoords.maxs = texCoords.mins + m_texCoordsPerTile;
	return texCoords;
}

//-----------------------------------------------------------------------------------
AABB2 SpriteSheet::GetTexCoordsForSpriteIndex(int spriteIndex) const
{
	AABB2 texCoords;
	int tileY = spriteIndex / m_spriteLayout.x;
	int tileX = spriteIndex % m_spriteLayout.x;

	//Because STBI is terrible and flips our images, we need to correct how textures are rendered.
	//Instead of (0,0) (1,1), we need to get the coordinates in (0,1) (1,0) order. This renders upright and correctly.
	//TODO: IF STBI GETS FIXED OR REPLACED, YOU'LL NEED TO REMOVE THIS AND MAKE IT MAKE MORE SENSE.
	Vector2 tempMins = Vector2(m_texCoordsPerTile.x * static_cast<float>(tileX), m_texCoordsPerTile.y * static_cast<float>(tileY));

	texCoords.maxs.x = tempMins.x + m_texCoordsPerTile.x;
	texCoords.maxs.y = tempMins.y;
	texCoords.mins.x = tempMins.x;
	texCoords.mins.y = tempMins.y + m_texCoordsPerTile.y;

	return texCoords;
}

//-----------------------------------------------------------------------------------
int SpriteSheet::GetNumSprites() const
{
	return m_spriteLayout.x * m_spriteLayout.y;
}

//-----------------------------------------------------------------------------------
Texture* SpriteSheet::GetTexture() const
{
	return m_spriteSheetTexture;
}
