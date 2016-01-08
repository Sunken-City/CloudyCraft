#pragma once

#include <map>
#include "GameCommon.hpp"
class SpriteSheet;
class Texture;

enum BlockType
{
	AIR = 0,
	STONE = 1,
	DIRT = 2,
	GRASS = 3,
	WATER = 4,
	SAND = 5,
	COBBLESTONE = 6,
	GLOWSTONE = 7,
	MOONSTONE = 8,
	MARBLE = 9,
	IRON = 10,
	OBSIDIAN = 11,
	REDSTONE,
	BLUESTONE,
	GREENSTONE,
	GRAY_GLASS,
	GREEN_GLASS,
	ORANGE_GLASS,
	NUM_BLOCKS
};

class BlockDefinition
{
public:
	static void Initialize();
	static Texture* GetTexture();
	static inline BlockDefinition* GetDefinition(uchar type);

	AABB2 GetTopIndex();
	AABB2 GetSideIndex();
	AABB2 GetBottomIndex();
	bool IsSolid();

	uchar m_topIndex;
	uchar m_sideIndex;
	uchar m_bottomIndex;
	RGBA m_opacity;
	unsigned int m_illumination;
	float m_toughness;
	bool m_isSolid;
	bool m_isOpaque;

	SoundID m_placeSound;
	SoundID m_brokenSound;

	static SpriteSheet* m_blockSheet;

private:
	static BlockDefinition s_definitionRegistry[BlockType::NUM_BLOCKS];
	BlockDefinition();
	~BlockDefinition();
};

inline BlockDefinition* BlockDefinition::GetDefinition(uchar type)
{
	return &(s_definitionRegistry[type]);
}