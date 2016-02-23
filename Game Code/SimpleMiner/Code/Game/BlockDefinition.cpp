#include "BlockDefinition.h"

BlockDefinition BlockDefinition::s_definitionRegistry[BlockType::NUM_BLOCKS];
SpriteSheet* BlockDefinition::m_blockSheet;

//-----------------------------------------------------------------------------------
void BlockDefinition::Initialize()
{
	m_blockSheet = new SpriteSheet("Data/Images/SimpleMinerAtlas.png", 16, 16);

	BlockDefinition air        = BlockDefinition();
	air.m_opacity              = RGBA(0x00000000);
	air.m_sideIndex            = 0x00;
	air.m_topIndex             = 0x00;
	air.m_bottomIndex          = 0x00;
	air.m_sideCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(air.m_sideIndex);
	air.m_bottomCoords         = m_blockSheet->GetTexCoordsForSpriteIndex(air.m_bottomIndex);
	air.m_topCoords            = m_blockSheet->GetTexCoordsForSpriteIndex(air.m_topIndex);
	air.m_isSolid              = false;
	air.m_isOpaque             = false;
	air.m_illumination         = 0x00000000;
	air.m_toughness            = 1.0f;
	air.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	air.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[AIR]  = air;

	BlockDefinition stone       = BlockDefinition();
	stone.m_opacity			    = RGBA(0xFFFFFF00);
	stone.m_sideIndex		    = 0xA2;
	stone.m_topIndex            = 0xA2;
	stone.m_bottomIndex         = 0xA2;
	stone.m_sideCoords          = m_blockSheet->GetTexCoordsForSpriteIndex(stone.m_sideIndex);
	stone.m_bottomCoords        = m_blockSheet->GetTexCoordsForSpriteIndex(stone.m_bottomIndex);
	stone.m_topCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(stone.m_topIndex);
	stone.m_isSolid             = true;
	stone.m_isOpaque            = true;
	stone.m_illumination        = 0x00000000;
	stone.m_toughness           = 2.5f;
	stone.m_placeSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	stone.m_brokenSound         = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[STONE] = stone;

	BlockDefinition dirt       = BlockDefinition();
	dirt.m_opacity			   = RGBA(0xFFFFFF00);
	dirt.m_sideIndex		   = 0x87;
	dirt.m_topIndex			   = 0x87;
	dirt.m_bottomIndex         = 0x87;
	dirt.m_sideCoords          = m_blockSheet->GetTexCoordsForSpriteIndex(dirt.m_sideIndex);
	dirt.m_bottomCoords        = m_blockSheet->GetTexCoordsForSpriteIndex(dirt.m_bottomIndex);
	dirt.m_topCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(dirt.m_topIndex);
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
	grass.m_bottomIndex         = 0x87;
	grass.m_sideCoords          = m_blockSheet->GetTexCoordsForSpriteIndex(grass.m_sideIndex);
	grass.m_bottomCoords        = m_blockSheet->GetTexCoordsForSpriteIndex(grass.m_bottomIndex);
	grass.m_topCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(grass.m_topIndex);
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
	water.m_sideCoords          = m_blockSheet->GetTexCoordsForSpriteIndex(water.m_sideIndex);
	water.m_bottomCoords        = m_blockSheet->GetTexCoordsForSpriteIndex(water.m_bottomIndex);
	water.m_topCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(water.m_topIndex);
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
	sand.m_sideCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(sand.m_sideIndex);
	sand.m_bottomCoords         = m_blockSheet->GetTexCoordsForSpriteIndex(sand.m_bottomIndex);
	sand.m_topCoords            = m_blockSheet->GetTexCoordsForSpriteIndex(sand.m_topIndex);
	sand.m_isSolid              = true;
	sand.m_isOpaque             = true;
	sand.m_illumination         = 0x00000000;
	sand.m_toughness            = 0.5f;
	sand.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digSand.ogg");
	sand.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digSand.ogg");
	s_definitionRegistry[SAND]  = sand;

	BlockDefinition cobblestone       = BlockDefinition();
	cobblestone.m_opacity             = RGBA(0xFFFFFF00);
	cobblestone.m_sideIndex           = 0xA5;
	cobblestone.m_topIndex            = 0xA5;
	cobblestone.m_bottomIndex         = 0xA5;
	cobblestone.m_sideCoords          = m_blockSheet->GetTexCoordsForSpriteIndex(cobblestone.m_sideIndex);
	cobblestone.m_bottomCoords        = m_blockSheet->GetTexCoordsForSpriteIndex(cobblestone.m_bottomIndex);
	cobblestone.m_topCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(cobblestone.m_topIndex);
	cobblestone.m_isSolid             = true;
	cobblestone.m_isOpaque            = true;
	cobblestone.m_illumination        = 0x00000000;
	cobblestone.m_toughness           = 2.5f;
	cobblestone.m_placeSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	cobblestone.m_brokenSound         = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[COBBLESTONE] = cobblestone;

	BlockDefinition obsidian       = BlockDefinition();
	obsidian.m_opacity             = RGBA(0xFFFFFF00);
	obsidian.m_sideIndex           = 0xB9;
	obsidian.m_topIndex            = 0xB9;
	obsidian.m_bottomIndex         = 0xB9;
	obsidian.m_sideCoords          = m_blockSheet->GetTexCoordsForSpriteIndex(obsidian.m_sideIndex);
	obsidian.m_bottomCoords        = m_blockSheet->GetTexCoordsForSpriteIndex(obsidian.m_bottomIndex);
	obsidian.m_topCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(obsidian.m_topIndex);
	obsidian.m_isSolid             = true;
	obsidian.m_isOpaque            = true;
	obsidian.m_illumination        = 0x00000000;
	obsidian.m_toughness           = 5.0f;
	obsidian.m_placeSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	obsidian.m_brokenSound         = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[OBSIDIAN] = obsidian;

	BlockDefinition iron       = BlockDefinition();
	iron.m_opacity             = RGBA(0xFFFFFF00);
	iron.m_sideIndex           = 0xBA;
	iron.m_topIndex            = 0xBA;
	iron.m_bottomIndex         = 0xBA;
	iron.m_sideCoords          = m_blockSheet->GetTexCoordsForSpriteIndex(iron.m_sideIndex);
	iron.m_bottomCoords        = m_blockSheet->GetTexCoordsForSpriteIndex(iron.m_bottomIndex);
	iron.m_topCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(iron.m_topIndex);
	iron.m_isSolid             = true;
	iron.m_isOpaque            = true;
	iron.m_illumination        = 0x00000000;
	iron.m_toughness           = 3.0f;
	iron.m_placeSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	iron.m_brokenSound         = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[IRON] = iron;

	BlockDefinition marble       = BlockDefinition();
	marble.m_opacity             = RGBA(0xFFFFFF00);
	marble.m_sideIndex           = 0xAB;
	marble.m_topIndex            = 0xAB;
	marble.m_bottomIndex         = 0xAB;
	marble.m_sideCoords          = m_blockSheet->GetTexCoordsForSpriteIndex(marble.m_sideIndex);
	marble.m_bottomCoords        = m_blockSheet->GetTexCoordsForSpriteIndex(marble.m_bottomIndex);
	marble.m_topCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(marble.m_topIndex);
	marble.m_isSolid             = true;
	marble.m_isOpaque            = true;
	marble.m_illumination        = 0x00000000;
	marble.m_toughness           = 3.0f;
	marble.m_placeSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	marble.m_brokenSound         = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[MARBLE] = marble;

	//LIGHTS//////////////////////////////////////////////////////////////////////////

	BlockDefinition glowstone          = BlockDefinition();
	glowstone.m_opacity                = RGBA(0xFFFFFF00);
	glowstone.m_sideIndex              = 0xB4;
	glowstone.m_topIndex               = 0xB4;
	glowstone.m_bottomIndex            = 0xB4;
	glowstone.m_sideCoords             = m_blockSheet->GetTexCoordsForSpriteIndex(glowstone.m_sideIndex);
	glowstone.m_bottomCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(glowstone.m_bottomIndex);
	glowstone.m_topCoords              = m_blockSheet->GetTexCoordsForSpriteIndex(glowstone.m_topIndex);
	glowstone.m_isSolid                = true;
	glowstone.m_isOpaque               = true;
	glowstone.m_illumination           = 0xDDEEFF00;
	glowstone.m_toughness              = 1.5f;
	glowstone.m_placeSound             = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	glowstone.m_brokenSound            = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[GLOWSTONE]    = glowstone;

	BlockDefinition moonstone          = BlockDefinition();
	moonstone.m_opacity                = RGBA(0xFFFFFF00);
	moonstone.m_sideIndex              = 0xB5;
	moonstone.m_topIndex               = 0xB5;
	moonstone.m_bottomIndex            = 0xB5;
	moonstone.m_sideCoords             = m_blockSheet->GetTexCoordsForSpriteIndex(moonstone.m_sideIndex);
	moonstone.m_bottomCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(moonstone.m_bottomIndex);
	moonstone.m_topCoords              = m_blockSheet->GetTexCoordsForSpriteIndex(moonstone.m_topIndex);
	moonstone.m_isSolid                = true;
	moonstone.m_isOpaque               = true;
	moonstone.m_illumination           = 0xCE98CD00;
	moonstone.m_toughness              = 1.5f;
	moonstone.m_placeSound             = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	moonstone.m_brokenSound            = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[MOONSTONE]    = moonstone;

	BlockDefinition whiteLight        = BlockDefinition();
	whiteLight.m_opacity              = RGBA(0xFFFFFF00);
	whiteLight.m_sideIndex            = 0x60;
	whiteLight.m_topIndex             = 0x60;
	whiteLight.m_bottomIndex          = 0x60;
	whiteLight.m_sideCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(whiteLight.m_sideIndex);
	whiteLight.m_bottomCoords         = m_blockSheet->GetTexCoordsForSpriteIndex(whiteLight.m_bottomIndex);
	whiteLight.m_topCoords            = m_blockSheet->GetTexCoordsForSpriteIndex(whiteLight.m_topIndex);
	whiteLight.m_isSolid              = true;
	whiteLight.m_isOpaque             = true;
	whiteLight.m_illumination         = 0xFFFFFF00;
	whiteLight.m_toughness            = 1.5f;
	whiteLight.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	whiteLight.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[WHITE_LIGHT] = whiteLight;

	BlockDefinition redLight        = BlockDefinition();
	redLight.m_opacity              = RGBA(0xFFFFFF00);
	redLight.m_sideIndex            = 0x61;
	redLight.m_topIndex             = 0x61;
	redLight.m_bottomIndex          = 0x61;
	redLight.m_sideCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(redLight.m_sideIndex);
	redLight.m_bottomCoords         = m_blockSheet->GetTexCoordsForSpriteIndex(redLight.m_bottomIndex);
	redLight.m_topCoords            = m_blockSheet->GetTexCoordsForSpriteIndex(redLight.m_topIndex);
	redLight.m_isSolid              = true;
	redLight.m_isOpaque             = true;
	redLight.m_illumination         = 0xFF000000;
	redLight.m_toughness            = 1.5f;
	redLight.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	redLight.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[RED_LIGHT] = redLight;

	BlockDefinition blueLight          = BlockDefinition();
	blueLight.m_opacity                = RGBA(0xFFFFFF00);
	blueLight.m_sideIndex              = 0x62;
	blueLight.m_topIndex               = 0x62;
	blueLight.m_bottomIndex            = 0x62;
	blueLight.m_sideCoords             = m_blockSheet->GetTexCoordsForSpriteIndex(blueLight.m_sideIndex);
	blueLight.m_bottomCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(blueLight.m_bottomIndex);
	blueLight.m_topCoords              = m_blockSheet->GetTexCoordsForSpriteIndex(blueLight.m_topIndex);
	blueLight.m_isSolid                = true;
	blueLight.m_isOpaque               = true;
	blueLight.m_illumination           = 0x0000FF00;
	blueLight.m_toughness              = 1.5f;
	blueLight.m_placeSound             = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	blueLight.m_brokenSound            = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[BLUE_LIGHT]   = blueLight;

	BlockDefinition greenLight        = BlockDefinition();
	greenLight.m_opacity              = RGBA(0xFFFFFF00);
	greenLight.m_sideIndex            = 0x63;
	greenLight.m_topIndex             = 0x63;
	greenLight.m_bottomIndex          = 0x63;
	greenLight.m_sideCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(greenLight.m_sideIndex);
	greenLight.m_bottomCoords         = m_blockSheet->GetTexCoordsForSpriteIndex(greenLight.m_bottomIndex);
	greenLight.m_topCoords            = m_blockSheet->GetTexCoordsForSpriteIndex(greenLight.m_topIndex);
	greenLight.m_isSolid              = true;
	greenLight.m_isOpaque             = true;
	greenLight.m_illumination         = 0x00FF0000;
	greenLight.m_toughness            = 1.5f;
	greenLight.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	greenLight.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[GREEN_LIGHT] = greenLight;

	BlockDefinition cyanLight        = BlockDefinition();
	cyanLight.m_opacity              = RGBA(0xFFFFFF00);
	cyanLight.m_sideIndex            = 0x64;
	cyanLight.m_topIndex             = 0x64;
	cyanLight.m_bottomIndex          = 0x64;
	cyanLight.m_sideCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(cyanLight.m_sideIndex);
	cyanLight.m_bottomCoords         = m_blockSheet->GetTexCoordsForSpriteIndex(cyanLight.m_bottomIndex);
	cyanLight.m_topCoords            = m_blockSheet->GetTexCoordsForSpriteIndex(cyanLight.m_topIndex);
	cyanLight.m_isSolid              = true;
	cyanLight.m_isOpaque             = true;
	cyanLight.m_illumination         = 0x00FFFF00;
	cyanLight.m_toughness            = 1.5f;
	cyanLight.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	cyanLight.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[CYAN_LIGHT] = cyanLight;

	BlockDefinition yellowLight        = BlockDefinition();
	yellowLight.m_opacity              = RGBA(0xFFFFFF00);
	yellowLight.m_sideIndex            = 0x65;
	yellowLight.m_topIndex             = 0x65;
	yellowLight.m_bottomIndex          = 0x65;
	yellowLight.m_sideCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(yellowLight.m_sideIndex);
	yellowLight.m_bottomCoords         = m_blockSheet->GetTexCoordsForSpriteIndex(yellowLight.m_bottomIndex);
	yellowLight.m_topCoords            = m_blockSheet->GetTexCoordsForSpriteIndex(yellowLight.m_topIndex);
	yellowLight.m_isSolid              = true;
	yellowLight.m_isOpaque             = true;
	yellowLight.m_illumination         = 0xFFFF0000;
	yellowLight.m_toughness            = 1.5f;
	yellowLight.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	yellowLight.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[YELLOW_LIGHT] = yellowLight;

	BlockDefinition magentaLight        = BlockDefinition();
	magentaLight.m_opacity              = RGBA(0xFFFFFF00);
	magentaLight.m_sideIndex            = 0x66;
	magentaLight.m_topIndex             = 0x66;
	magentaLight.m_bottomIndex          = 0x66;
	magentaLight.m_sideCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(magentaLight.m_sideIndex);
	magentaLight.m_bottomCoords         = m_blockSheet->GetTexCoordsForSpriteIndex(magentaLight.m_bottomIndex);
	magentaLight.m_topCoords            = m_blockSheet->GetTexCoordsForSpriteIndex(magentaLight.m_topIndex);
	magentaLight.m_isSolid              = true;
	magentaLight.m_isOpaque             = true;
	magentaLight.m_illumination         = 0xFF00FF00;
	magentaLight.m_toughness            = 1.5f;
	magentaLight.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	magentaLight.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[MAGENTA_LIGHT] = magentaLight;

	BlockDefinition orangeLight        = BlockDefinition();
	orangeLight.m_opacity              = RGBA(0xFFFFFF00);
	orangeLight.m_sideIndex            = 0x67;
	orangeLight.m_topIndex             = 0x67;
	orangeLight.m_bottomIndex          = 0x67;
	orangeLight.m_sideCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(orangeLight.m_sideIndex);
	orangeLight.m_bottomCoords         = m_blockSheet->GetTexCoordsForSpriteIndex(orangeLight.m_bottomIndex);
	orangeLight.m_topCoords            = m_blockSheet->GetTexCoordsForSpriteIndex(orangeLight.m_topIndex);
	orangeLight.m_isSolid              = true;
	orangeLight.m_isOpaque             = true;
	orangeLight.m_illumination         = 0xFF990000;
	orangeLight.m_toughness            = 1.5f;
	orangeLight.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	orangeLight.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[ORANGE_LIGHT] = orangeLight;

	BlockDefinition grayLight        = BlockDefinition();
	grayLight.m_opacity              = RGBA(0xFFFFFF00);
	grayLight.m_sideIndex            = 0x68;
	grayLight.m_topIndex             = 0x68;
	grayLight.m_bottomIndex          = 0x68;
	grayLight.m_sideCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(grayLight.m_sideIndex);
	grayLight.m_bottomCoords         = m_blockSheet->GetTexCoordsForSpriteIndex(grayLight.m_bottomIndex);
	grayLight.m_topCoords            = m_blockSheet->GetTexCoordsForSpriteIndex(grayLight.m_topIndex);
	grayLight.m_isSolid              = true;
	grayLight.m_isOpaque             = true;
	grayLight.m_illumination         = 0x7F7F7F00;
	grayLight.m_toughness            = 1.5f;
	grayLight.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	grayLight.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[GRAY_LIGHT] = grayLight;

	BlockDefinition purpleLight        = BlockDefinition();
	purpleLight.m_opacity              = RGBA(0xFFFFFF00);
	purpleLight.m_sideIndex            = 0x69;
	purpleLight.m_topIndex             = 0x69;
	purpleLight.m_bottomIndex          = 0x69;
	purpleLight.m_sideCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(purpleLight.m_sideIndex);
	purpleLight.m_bottomCoords         = m_blockSheet->GetTexCoordsForSpriteIndex(purpleLight.m_bottomIndex);
	purpleLight.m_topCoords            = m_blockSheet->GetTexCoordsForSpriteIndex(purpleLight.m_topIndex);
	purpleLight.m_isSolid              = true;
	purpleLight.m_isOpaque             = true;
	purpleLight.m_illumination         = 0x7F00FF00;
	purpleLight.m_toughness            = 1.5f;
	purpleLight.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	purpleLight.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[PURPLE_LIGHT] = purpleLight;

	//GLASS//////////////////////////////////////////////////////////////////////////

	BlockDefinition whiteGlass        = BlockDefinition();
	whiteGlass.m_opacity              = RGBA(0x00000000);
	whiteGlass.m_sideIndex            = 0x70;
	whiteGlass.m_topIndex             = 0x70;
	whiteGlass.m_bottomIndex          = 0x70;
	whiteGlass.m_sideCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(whiteGlass.m_sideIndex);
	whiteGlass.m_bottomCoords         = m_blockSheet->GetTexCoordsForSpriteIndex(whiteGlass.m_bottomIndex);
	whiteGlass.m_topCoords            = m_blockSheet->GetTexCoordsForSpriteIndex(whiteGlass.m_topIndex);
	whiteGlass.m_isSolid              = true;
	whiteGlass.m_isOpaque             = false;
	whiteGlass.m_illumination         = 0x00000000;
	whiteGlass.m_toughness            = 0.5f;
	whiteGlass.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	whiteGlass.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[WHITE_GLASS] = whiteGlass;

	BlockDefinition redGlass        = BlockDefinition();
	redGlass.m_opacity              = RGBA(0x00FFFF00);
	redGlass.m_sideIndex            = 0x71;
	redGlass.m_topIndex             = 0x71;
	redGlass.m_bottomIndex          = 0x71;
	redGlass.m_sideCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(redGlass.m_sideIndex);
	redGlass.m_bottomCoords         = m_blockSheet->GetTexCoordsForSpriteIndex(redGlass.m_bottomIndex);
	redGlass.m_topCoords            = m_blockSheet->GetTexCoordsForSpriteIndex(redGlass.m_topIndex);
	redGlass.m_isSolid              = true;
	redGlass.m_isOpaque             = false;
	redGlass.m_illumination         = 0x00000000;
	redGlass.m_toughness            = 0.5f;
	redGlass.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	redGlass.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[RED_GLASS] = redGlass;

	BlockDefinition blueGlass        = BlockDefinition();
	blueGlass.m_opacity              = RGBA(0xFFFF0000);
	blueGlass.m_sideIndex            = 0x72;
	blueGlass.m_topIndex             = 0x72;
	blueGlass.m_bottomIndex          = 0x72;
	blueGlass.m_sideCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(blueGlass.m_sideIndex);
	blueGlass.m_bottomCoords         = m_blockSheet->GetTexCoordsForSpriteIndex(blueGlass.m_bottomIndex);
	blueGlass.m_topCoords            = m_blockSheet->GetTexCoordsForSpriteIndex(blueGlass.m_topIndex);
	blueGlass.m_isSolid              = true;
	blueGlass.m_isOpaque             = false;
	blueGlass.m_illumination         = 0x00000000;
	blueGlass.m_toughness            = 0.5f;
	blueGlass.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	blueGlass.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[BLUE_GLASS] = blueGlass;

	BlockDefinition greenGlass        = BlockDefinition();
	greenGlass.m_opacity              = RGBA(0xFF00FF00);
	greenGlass.m_sideIndex            = 0x73;
	greenGlass.m_topIndex             = 0x73;
	greenGlass.m_bottomIndex          = 0x73;
	greenGlass.m_sideCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(greenGlass.m_sideIndex);
	greenGlass.m_bottomCoords         = m_blockSheet->GetTexCoordsForSpriteIndex(greenGlass.m_bottomIndex);
	greenGlass.m_topCoords            = m_blockSheet->GetTexCoordsForSpriteIndex(greenGlass.m_topIndex);
	greenGlass.m_isSolid              = true;
	greenGlass.m_isOpaque             = false;
	greenGlass.m_illumination         = 0x00000000;
	greenGlass.m_toughness            = 0.5f;
	greenGlass.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	greenGlass.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[GREEN_GLASS] = greenGlass;

	BlockDefinition cyanGlass        = BlockDefinition();
	cyanGlass.m_opacity              = RGBA(0xFF0000FF);
	cyanGlass.m_sideIndex            = 0x74;
	cyanGlass.m_topIndex             = 0x74;
	cyanGlass.m_bottomIndex          = 0x74;
	cyanGlass.m_sideCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(cyanGlass.m_sideIndex);
	cyanGlass.m_bottomCoords         = m_blockSheet->GetTexCoordsForSpriteIndex(cyanGlass.m_bottomIndex);
	cyanGlass.m_topCoords            = m_blockSheet->GetTexCoordsForSpriteIndex(cyanGlass.m_topIndex);
	cyanGlass.m_isSolid              = true;
	cyanGlass.m_isOpaque             = false;
	cyanGlass.m_illumination         = 0x00000000;
	cyanGlass.m_toughness            = 0.5f;
	cyanGlass.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	cyanGlass.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[CYAN_GLASS] = cyanGlass;

	BlockDefinition yellowGlass        = BlockDefinition();
	yellowGlass.m_opacity              = RGBA(0x0000FF00);
	yellowGlass.m_sideIndex            = 0x75;
	yellowGlass.m_topIndex             = 0x75;
	yellowGlass.m_bottomIndex          = 0x75;
	yellowGlass.m_sideCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(yellowGlass.m_sideIndex);
	yellowGlass.m_bottomCoords         = m_blockSheet->GetTexCoordsForSpriteIndex(yellowGlass.m_bottomIndex);
	yellowGlass.m_topCoords            = m_blockSheet->GetTexCoordsForSpriteIndex(yellowGlass.m_topIndex);
	yellowGlass.m_isSolid              = true;
	yellowGlass.m_isOpaque             = false;
	yellowGlass.m_illumination         = 0x00000000;
	yellowGlass.m_toughness            = 0.5f;
	yellowGlass.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	yellowGlass.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[YELLOW_GLASS] = yellowGlass;

	BlockDefinition magentaGlass        = BlockDefinition();
	magentaGlass.m_opacity              = RGBA(0x00FF0000);
	magentaGlass.m_sideIndex            = 0x76;
	magentaGlass.m_topIndex             = 0x76;
	magentaGlass.m_bottomIndex          = 0x76;
	magentaGlass.m_sideCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(magentaGlass.m_sideIndex);
	magentaGlass.m_bottomCoords         = m_blockSheet->GetTexCoordsForSpriteIndex(magentaGlass.m_bottomIndex);
	magentaGlass.m_topCoords            = m_blockSheet->GetTexCoordsForSpriteIndex(magentaGlass.m_topIndex);
	magentaGlass.m_isSolid              = true;
	magentaGlass.m_isOpaque             = false;
	magentaGlass.m_illumination         = 0x00000000;
	magentaGlass.m_toughness            = 0.5f;
	magentaGlass.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	magentaGlass.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[MAGENTA_GLASS] = magentaGlass;

	BlockDefinition orangeGlass        = BlockDefinition();
	orangeGlass.m_opacity              = RGBA(0x0066FF00); //0xFFFFFF(White) - 0xFF9900(Orange)
	orangeGlass.m_sideIndex            = 0x77;
	orangeGlass.m_topIndex             = 0x77;
	orangeGlass.m_bottomIndex          = 0x77;
	orangeGlass.m_sideCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(orangeGlass.m_sideIndex);
	orangeGlass.m_bottomCoords         = m_blockSheet->GetTexCoordsForSpriteIndex(orangeGlass.m_bottomIndex);
	orangeGlass.m_topCoords            = m_blockSheet->GetTexCoordsForSpriteIndex(orangeGlass.m_topIndex);
	orangeGlass.m_isSolid              = true;
	orangeGlass.m_isOpaque             = false;
	orangeGlass.m_illumination         = 0x00000000;
	orangeGlass.m_toughness            = 0.5f;
	orangeGlass.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	orangeGlass.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[ORANGE_GLASS] = orangeGlass;

	BlockDefinition greyGlass        = BlockDefinition();
	greyGlass.m_opacity              = RGBA(0x80808000); //0xff - 0x7f = 0x80
	greyGlass.m_sideIndex            = 0x78;
	greyGlass.m_topIndex             = 0x78;
	greyGlass.m_bottomIndex          = 0x78;
	greyGlass.m_sideCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(greyGlass.m_sideIndex);
	greyGlass.m_bottomCoords         = m_blockSheet->GetTexCoordsForSpriteIndex(greyGlass.m_bottomIndex);
	greyGlass.m_topCoords            = m_blockSheet->GetTexCoordsForSpriteIndex(greyGlass.m_topIndex);
	greyGlass.m_isSolid              = true;
	greyGlass.m_isOpaque             = false;
	greyGlass.m_illumination         = 0x00000000;
	greyGlass.m_toughness            = 0.5f;
	greyGlass.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	greyGlass.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[GRAY_GLASS] = greyGlass;

	BlockDefinition purpleGlass        = BlockDefinition();
	purpleGlass.m_opacity              = RGBA(0x80FF0000); //0xFFFFFF - 0x7f00ff
	purpleGlass.m_sideIndex            = 0x79;
	purpleGlass.m_topIndex             = 0x79;
	purpleGlass.m_bottomIndex          = 0x79;
	purpleGlass.m_sideCoords           = m_blockSheet->GetTexCoordsForSpriteIndex(purpleGlass.m_sideIndex);
	purpleGlass.m_bottomCoords         = m_blockSheet->GetTexCoordsForSpriteIndex(purpleGlass.m_bottomIndex);
	purpleGlass.m_topCoords            = m_blockSheet->GetTexCoordsForSpriteIndex(purpleGlass.m_topIndex);
	purpleGlass.m_isSolid              = true;
	purpleGlass.m_isOpaque             = false;
	purpleGlass.m_illumination         = 0x00000000;
	purpleGlass.m_toughness            = 0.5f;
	purpleGlass.m_placeSound           = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	purpleGlass.m_brokenSound          = AudioSystem::instance->CreateOrGetSound("Data/SFX/Minecraft/digStone.ogg");
	s_definitionRegistry[PURPLE_GLASS] = purpleGlass;
}

//-----------------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------------
BlockDefinition::~BlockDefinition()
{
}
