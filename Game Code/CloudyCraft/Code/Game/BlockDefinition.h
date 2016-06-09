#pragma once

#include <map>
#include "GameCommon.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
class Texture;

//ENUMS//////////////////////////////////////////////////////////////////////////
enum BlockType
{
    AIR = 0,
    STONE,
    DIRT,
    GRASS,
    WATER,
    SAND,
    COBBLESTONE,
    MARBLE,
    IRON,
    OBSIDIAN,
    //Lights//////////////
    GLOWSTONE,
    MOONSTONE,
    WHITE_LIGHT,
    RED_LIGHT,
    BLUE_LIGHT,
    GREEN_LIGHT,
    CYAN_LIGHT,
    YELLOW_LIGHT,
    MAGENTA_LIGHT,
    ORANGE_LIGHT,
    PURPLE_LIGHT,
    GRAY_LIGHT,
    //Glass///////////////
    WHITE_GLASS,
    RED_GLASS,
    BLUE_GLASS,
    GREEN_GLASS,
    CYAN_GLASS,
    YELLOW_GLASS,
    MAGENTA_GLASS,
    ORANGE_GLASS,
    PURPLE_GLASS,
    GRAY_GLASS,
    NUM_BLOCKS
};

class BlockDefinition
{
public:
    //FUNCTIONS//////////////////////////////////////////////////////////////////////////
    static void Initialize();
    static inline Texture* GetTexture() { return m_blockSheet->GetTexture(); };
    static inline BlockDefinition* GetDefinition(uchar type) { return &(s_definitionRegistry[type]); };
    static void Uninitialize();

    inline AABB2 GetTopIndex() { return m_topCoords; };
    inline AABB2 GetSideIndex() { return m_sideCoords; };
    inline AABB2 GetBottomIndex() { return m_bottomCoords; };
    inline bool IsSolid() { return m_isSolid; };
    inline bool IsIlluminated() { return m_illumination > 0xFF; }; //If it's larger than the max alpha value, it's illuminated somewhat.

    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    static SpriteSheet* m_blockSheet;

    uchar m_topIndex;
    uchar m_sideIndex;
    uchar m_bottomIndex;
    AABB2 m_topCoords;
    AABB2 m_sideCoords;
    AABB2 m_bottomCoords;
    RGBA m_opacity;
    SoundID m_placeSound;
    SoundID m_brokenSound;
    unsigned int m_illumination;
    float m_toughness;
    bool m_isSolid;
    bool m_isOpaque;

private:
    static BlockDefinition s_definitionRegistry[BlockType::NUM_BLOCKS];

    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    BlockDefinition();
    ~BlockDefinition();
};