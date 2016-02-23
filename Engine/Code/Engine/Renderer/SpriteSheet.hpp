#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector2Int.hpp"
#include "Engine/Renderer/Texture.hpp"

class AABB2;

class SpriteSheet
{
public:
	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
	SpriteSheet(const std::string& imageFilePath, int tilesWide, int tilesHigh);

	//GETTERS//////////////////////////////////////////////////////////////////////////
	AABB2 GetTexCoordsForSpriteCoords(const Vector2Int& spriteCoords) const; // mostly for atlases
	AABB2 GetTexCoordsForSpriteIndex(int spriteIndex) const; // mostly for sprite animations
	int GetNumSprites() const;
	Texture* GetTexture() const;

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
private:
	Texture* 	m_spriteSheetTexture;	// Image with grid-based layout of sub-images
	Vector2		m_texCoordsPerTile; //One step of tile in tile coords
	Vector2Int	m_spriteLayout;	// # of sprites across, and down, on the sheet
};
