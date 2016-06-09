#pragma once
#include <vector>
#include "Engine/Audio/Audio.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Core/ProfilingUtils.h"

class SpriteSheet;
class RGBA;
class Camera3D;
class World;
class Player;
class Material;
class Framebuffer;

//GLOBALS//////////////////////////////////////////////////////////////////////////
extern ProfilingID g_generationProfiling;
extern ProfilingID g_loadingProfiling;
extern ProfilingID g_savingProfiling;
extern ProfilingID g_vaBuildingProfiling;
extern ProfilingID g_temporaryProfiling;

class TheGame
{
public:
    TheGame();
    ~TheGame();
    void Update(float deltaTime);
    void Begin3DPerspective() const;
    void End3DPerspective() const;
    void Render() const;

    void DebugRender() const;
    void RenderUI() const;
    void RenderDebugText() const;
    void RenderAxisLines() const;
    void RenderCrosshair() const;
    void RenderInventory() const;
    void SwapWorlds();

    static TheGame* instance;

    SpriteSheet* m_blockSheet;
    Camera3D* m_playerCamera;
    Player* m_player;
    std::vector<World*> m_worlds;
    Framebuffer* m_primaryWorldFramebuffer;
    Framebuffer* m_secondaryWorldFramebuffer;
    Material* m_blockMaterial;
    Material* m_blockMaterialWithoutPortals;
    SoundID m_worldSwapSFX;
    unsigned int m_currentlyRenderedWorldID;
    unsigned int m_alternateRenderedWorldID;

private:
    //Ignore warnings for being unable to generate a copy constructor for singleton class.
    TheGame& operator= (const TheGame& other);
    void UpdateDebug();
};
