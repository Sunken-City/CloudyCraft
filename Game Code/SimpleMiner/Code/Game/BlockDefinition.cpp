#include "BlockDefinition.h"
#include "Engine/Renderer/SpriteSheet.hpp"

BlockDefinition BlockDefinition::s_definitionRegistry[BlockType::NUM_BLOCKS];
SpriteSheet* BlockDefinition::m_blockSheet;

void BlockDefinition::Initialize()
{
	m_blockSheet = new SpriteSheet("Data/Images/SimpleMinerAtlas.png", 16, 16);

	BlockDefinition air       = BlockDefinition();
	air.m_opacity             = RGBA(0x00000000);
	air.m_sideIndex           = 0x00;
	air.m_topIndex            = 0x00;
	air.m_bottomIndex         = 0x00;
	air.m_isSolid             = false;
	air.m_isOpaque            = false;
	air.m_illumination        = 0x00000000;
	air.m_toughness           = 1.0f;
	air.m_placeSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	air.m_brokenSound         = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[AIR] = air;

	BlockDefinition stone       = BlockDefinition();
	stone.m_opacity			    = RGBA(0xFFFFFF00);
	stone.m_sideIndex		    = 0xA2;
	stone.m_topIndex		    = 0xA2;
	stone.m_bottomIndex		    = 0xA2;
	stone.m_isSolid			    = true;
	stone.m_isOpaque		    = true;
	stone.m_illumination	    = 0x00000000;
	stone.m_toughness           = 2.5f;
	stone.m_placeSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	stone.m_brokenSound         = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[STONE] = stone;

	BlockDefinition dirt       = BlockDefinition();
	dirt.m_opacity			   = RGBA(0xFFFFFF00);
	dirt.m_sideIndex		   = 0x87;
	dirt.m_topIndex			   = 0x87;
	dirt.m_bottomIndex		   = 0x87;
	dirt.m_isSolid			   = true;
	dirt.m_isOpaque			   = true;
	dirt.m_illumination		   = 0x00000000;
	dirt.m_toughness           = 0.5f;
	dirt.m_placeSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digGrass.ogg");
	dirt.m_brokenSound         = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digGrass.ogg");
	s_definitionRegistry[DIRT] = dirt;
	
	BlockDefinition grass       = BlockDefinition();
	grass.m_opacity			    = RGBA(0xFFFFFF00);
	grass.m_sideIndex		    = 0x88;
	grass.m_topIndex			= 0x89;
	grass.m_bottomIndex		    = 0x87;
	grass.m_isSolid			    = true;
	grass.m_isOpaque			= true;
	grass.m_illumination		= 0x00000000;
	grass.m_toughness           = 0.5f;
	grass.m_placeSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digGrass.ogg");
	grass.m_brokenSound         = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digGrass.ogg");
	s_definitionRegistry[GRASS] = grass;

	BlockDefinition water       = BlockDefinition();
	water.m_opacity             = RGBA(0x33333300);
	water.m_sideIndex           = 0xBF; 
	water.m_topIndex            = 0xBF;
	water.m_bottomIndex         = 0xBF;
	water.m_isSolid             = false;
	water.m_isOpaque            = false;
	water.m_illumination        = 0x00000000;
	water.m_toughness           = 0.0f;
	water.m_placeSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	water.m_brokenSound         = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[WATER] = water;

	BlockDefinition sand        = BlockDefinition();
	sand.m_opacity              = RGBA(0xFFFFFF00);
	sand.m_sideIndex            = 0x81;
	sand.m_topIndex             = 0x81;
	sand.m_bottomIndex          = 0x81;
	sand.m_isSolid              = true;
	sand.m_isOpaque             = true;
	sand.m_illumination         = 0x00000000;
	sand.m_toughness            = 0.5f;
	sand.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digSand.ogg");
	sand.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digSand.ogg");
	s_definitionRegistry[SAND]  = sand;

	BlockDefinition cobblestone            = BlockDefinition();
	cobblestone.m_opacity                  = RGBA(0xFFFFFF00);
	cobblestone.m_sideIndex                = 0xA5;
	cobblestone.m_topIndex                 = 0xA5;
	cobblestone.m_bottomIndex              = 0xA5;
	cobblestone.m_isSolid                  = true;
	cobblestone.m_isOpaque                 = true;
	cobblestone.m_illumination             = 0x00000000;
	cobblestone.m_toughness                = 2.5f;
	cobblestone.m_placeSound               = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	cobblestone.m_brokenSound              = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[COBBLESTONE]      = cobblestone;

	BlockDefinition glowstone          = BlockDefinition();
	glowstone.m_opacity                = RGBA(0xFFFFFF00);
	glowstone.m_sideIndex              = 0xB4;
	glowstone.m_topIndex               = 0xB4;
	glowstone.m_bottomIndex            = 0xB4;
	glowstone.m_isSolid                = true;
	glowstone.m_isOpaque               = true;
	glowstone.m_illumination           = 0xCCCCCC00;
	glowstone.m_toughness              = 1.5f;
	glowstone.m_placeSound             = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	glowstone.m_brokenSound            = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[GLOWSTONE]    = glowstone;

	BlockDefinition moonstone          = BlockDefinition();
	moonstone.m_opacity                = RGBA(0xFFFFFF00);
	moonstone.m_sideIndex              = 0xB5;
	moonstone.m_topIndex               = 0xB5;
	moonstone.m_bottomIndex            = 0xB5;
	moonstone.m_isSolid                = true;
	moonstone.m_isOpaque               = true;
	moonstone.m_illumination           = 0xCE98CD00;
	moonstone.m_toughness              = 1.5f;
	moonstone.m_placeSound             = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	moonstone.m_brokenSound            = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[MOONSTONE]    = moonstone;

	BlockDefinition redstone         = BlockDefinition();
	redstone.m_opacity               = RGBA(0xFFFFFF00);
	redstone.m_sideIndex             = 0x07;
	redstone.m_topIndex              = 0x07;
	redstone.m_bottomIndex           = 0x07;
	redstone.m_isSolid               = true;
	redstone.m_isOpaque              = true;
	redstone.m_illumination          = 0xFF000000;
	redstone.m_toughness             = 1.5f;
	redstone.m_placeSound            = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	redstone.m_brokenSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[REDSTONE]   = redstone;

	BlockDefinition greenstone           = BlockDefinition();
	greenstone.m_opacity                 = RGBA(0xFFFFFF00);
	greenstone.m_sideIndex               = 0x16;
	greenstone.m_topIndex                = 0x16;
	greenstone.m_bottomIndex             = 0x16;
	greenstone.m_isSolid                 = true;
	greenstone.m_isOpaque                = true;
	greenstone.m_illumination            = 0x00FF0000;
	greenstone.m_toughness               = 1.5f;
	greenstone.m_placeSound              = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	greenstone.m_brokenSound             = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[GREENSTONE]     = greenstone;

	BlockDefinition bluestone          = BlockDefinition();
	bluestone.m_opacity                = RGBA(0xFFFFFF00);
	bluestone.m_sideIndex              = 0x06;
	bluestone.m_topIndex               = 0x06;
	bluestone.m_bottomIndex            = 0x06;
	bluestone.m_isSolid                = true;
	bluestone.m_isOpaque               = true;
	bluestone.m_illumination           = 0x0000FF00;
	bluestone.m_toughness              = 1.5f;
	bluestone.m_placeSound             = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	bluestone.m_brokenSound            = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[BLUESTONE]    = bluestone;

	BlockDefinition obsidian         = BlockDefinition();
	obsidian.m_opacity               = RGBA(0xFFFFFF00);
	obsidian.m_sideIndex             = 0xB9;
	obsidian.m_topIndex              = 0xB9;
	obsidian.m_bottomIndex           = 0xB9;
	obsidian.m_isSolid               = true;
	obsidian.m_isOpaque              = true;
	obsidian.m_illumination          = 0x00000000;
	obsidian.m_toughness             = 5.0f;
	obsidian.m_placeSound            = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	obsidian.m_brokenSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[OBSIDIAN]   = obsidian;

	BlockDefinition iron        = BlockDefinition();
	iron.m_opacity              = RGBA(0xFFFFFF00);
	iron.m_sideIndex            = 0xBA;
	iron.m_topIndex             = 0xBA;
	iron.m_bottomIndex          = 0xBA;
	iron.m_isSolid              = true;
	iron.m_isOpaque             = true;
	iron.m_illumination         = 0x00000000;
	iron.m_toughness            = 3.0f;
	iron.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	iron.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[IRON]  = iron;

	BlockDefinition marble        = BlockDefinition();
	marble.m_opacity              = RGBA(0xFFFFFF00);
	marble.m_sideIndex            = 0xAB;
	marble.m_topIndex             = 0xAB;
	marble.m_bottomIndex          = 0xAB;
	marble.m_isSolid              = true;
	marble.m_isOpaque             = true;
	marble.m_illumination         = 0x00000000;
	marble.m_toughness            = 3.0f;
	marble.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	marble.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[MARBLE]  = marble;

	BlockDefinition orangeGlass        = BlockDefinition();
	orangeGlass.m_opacity              = RGBA(0x0066FF00); //0xFFFFFF - 0xFF9900
	orangeGlass.m_sideIndex            = 0x76;
	orangeGlass.m_topIndex             = 0x76;
	orangeGlass.m_bottomIndex          = 0x76;
	orangeGlass.m_isSolid              = true;
	orangeGlass.m_isOpaque             = false;
	orangeGlass.m_illumination         = 0x00000000;
	orangeGlass.m_toughness            = 0.5f;
	orangeGlass.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	orangeGlass.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[ORANGE_GLASS] = orangeGlass;

	BlockDefinition greyGlass        = BlockDefinition();
	greyGlass.m_opacity              = RGBA(0x66666600);
	greyGlass.m_sideIndex            = 0x77;
	greyGlass.m_topIndex             = 0x77;
	greyGlass.m_bottomIndex          = 0x77;
	greyGlass.m_isSolid              = true;
	greyGlass.m_isOpaque             = false;
	greyGlass.m_illumination         = 0x00000000;
	greyGlass.m_toughness            = 0.5f;
	greyGlass.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	greyGlass.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[GRAY_GLASS] = greyGlass;

	BlockDefinition greenGlass        = BlockDefinition();
	greenGlass.m_opacity              = RGBA(0xFF00FF00);
	greenGlass.m_sideIndex            = 0x71;
	greenGlass.m_topIndex             = 0x71;
	greenGlass.m_bottomIndex          = 0x71;
	greenGlass.m_isSolid              = true;
	greenGlass.m_isOpaque             = false;
	greenGlass.m_illumination         = 0x00000000;
	greenGlass.m_toughness            = 0.5f;
	greenGlass.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	greenGlass.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[GREEN_GLASS] = greenGlass;
}

BlockDefinition::BlockDefinition()
: m_opacity(RGBA(0xFFFFFF00))
, m_sideIndex(0x00)
, m_topIndex(0x00)
, m_bottomIndex(0x00)
, m_isSolid(false)
, m_isOpaque(false)
, m_illumination(0x00000000)
, m_toughness(1.0f)
{

}

Texture* BlockDefinition::GetTexture()
{
	return m_blockSheet->GetTexture();
}

AABB2 BlockDefinition::GetTopIndex()
{
	return m_blockSheet->GetTexCoordsForSpriteIndex(m_topIndex);
}

AABB2 BlockDefinition::GetSideIndex()
{
	return m_blockSheet->GetTexCoordsForSpriteIndex(m_sideIndex);
}

AABB2 BlockDefinition::GetBottomIndex()
{
	return m_blockSheet->GetTexCoordsForSpriteIndex(m_bottomIndex);
}

BlockDefinition::~BlockDefinition()
{
}

bool BlockDefinition::IsSolid()
{
	return m_isSolid;
}
