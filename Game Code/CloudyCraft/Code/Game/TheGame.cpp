#include "Game/TheGame.hpp"
#include "Game/TheApp.hpp"
#include "Game/Camera3D.hpp"
#include "Game/World.hpp"
#include "Game/BlockDefinition.h"
#include "Game/Player.hpp"
#include "Game/Generator.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Vertex.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Renderer/AABB3.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Input/XInputController.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/Console.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/MatrixStack4x4.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/Framebuffer.hpp"
#include <thread>
#include <regex>

TheGame* TheGame::instance = nullptr;
ProfilingID g_generationProfiling;
ProfilingID g_loadingProfiling;
ProfilingID g_savingProfiling;
ProfilingID g_vaBuildingProfiling;
ProfilingID g_temporaryProfiling;

//-----------------------------------------------------------------------------------
void TheGame::UpdateDebug()
{
    g_frameNumber++;
    if (InputSystem::instance->WasKeyJustPressed('Q'))
    {
        ResetStats();
    }
}

//-----------------------------------------------------------------------------------
TheGame::TheGame()
    : m_blockSheet(new SpriteSheet("Data/Images/SimpleMinerAtlas.png", 16, 16))
    , m_currentlyRenderedWorldID(0)
    , m_alternateRenderedWorldID(1)
    , m_worldSwapSFX(AudioSystem::instance->CreateOrGetSound("Data/SFX/swapDimensions.wav"))
    , m_blockMaterial(new Material(new ShaderProgram("Data/Shaders/fvfPortal.vert", "Data/Shaders/fvfPortal.frag"),
    RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)))
    , m_blockMaterialWithoutPortals(new Material(new ShaderProgram("Data/Shaders/fixedVertexFormat.vert", "Data/Shaders/fixedVertexFormat.frag"),
        RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)))
    , m_primaryWorldFramebuffer(nullptr)
    , m_secondaryWorldFramebuffer(nullptr)
    , m_primaryWorldFBOMaterial(new Material(new ShaderProgram("Data/Shaders/Post/post.vert", "Data/Shaders/Post/post_tritanopia.frag"),
        RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::RENDER_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)))
{
    g_generationProfiling = RegisterProfilingChannel();
    g_loadingProfiling = RegisterProfilingChannel();
    g_savingProfiling = RegisterProfilingChannel();
    g_vaBuildingProfiling = RegisterProfilingChannel();
    g_temporaryProfiling = RegisterProfilingChannel();

    BlockDefinition::Initialize();
    m_worlds.push_back(new World(0, RGBA(0xDDEEFFFF), RGBA(0x4DC9FFFF), new EarthGenerator()));			//BlueSky 0x4DC9FFFF     Vaporwave 0xFF819CFF
    m_worlds.push_back(new World(1, RGBA(0xFDDA0EFF), RGBA(0xC55409FF), new SkylandsGenerator()));
    //Why does this have to be here? I had it in initializer list, but caused race condition. Reminder to ask someone.
    m_player = new Player(m_worlds[0]);
    m_playerCamera = &(m_player->m_camera);
    for (World* world : m_worlds)
    {
        world->CreateRenderingOffsetList(m_player);
    }

    //Console startup message
    Console::instance->PrintLine("Welcome to CloudyCraft", RGBA::GOLD);
    Console::instance->RunCommand("help");

    Texture* colorTargetsFirstPass[2];
    colorTargetsFirstPass[0] = new Texture(1600, 900, Texture::TextureFormat::RGBA8);
    colorTargetsFirstPass[1] = new Texture(1600, 900, Texture::TextureFormat::R32UI);
    Texture* depthTex1 = new Texture(1600, 900, Texture::TextureFormat::D24S8);
    m_primaryWorldFramebuffer = Framebuffer::FramebufferCreate(2, colorTargetsFirstPass, depthTex1);

    Texture* colorTargetsSecondPass[2];
    colorTargetsSecondPass[0] = colorTargetsFirstPass[0];
    colorTargetsSecondPass[1] = new Texture(1600, 900, Texture::TextureFormat::R32UI);
    Texture* depthTex2 = new Texture(1600, 900, Texture::TextureFormat::D24S8);
    m_secondaryWorldFramebuffer = Framebuffer::FramebufferCreate(2, colorTargetsSecondPass, depthTex2);

    m_blockMaterial->SetDiffuseTexture(m_blockSheet->GetTexture());
    m_blockMaterial->SetNormalTexture(Texture::CreateOrGetTexture("Data/Images/PortalMap.png"));
    m_blockMaterialWithoutPortals->SetDiffuseTexture(m_blockSheet->GetTexture());
}

//-----------------------------------------------------------------------------------
TheGame::~TheGame()
{
    BlockDefinition::Uninitialize();
    for (World* world : m_worlds)
    {
        delete world;
    }
    m_worlds.clear();
    delete m_player;
    delete m_blockSheet;
    delete m_blockMaterial->m_shaderProgram;
    delete m_blockMaterial;
    delete m_blockMaterialWithoutPortals->m_shaderProgram;
    delete m_blockMaterialWithoutPortals;
    delete m_primaryWorldFBOMaterial->m_shaderProgram;
    delete m_primaryWorldFBOMaterial;
    delete m_primaryWorldFramebuffer->m_depthStencilTarget;
    for (unsigned int i = 0; i < m_primaryWorldFramebuffer->m_colorCount; ++i)
    {
        delete m_primaryWorldFramebuffer->m_colorTargets[i];
    }
    delete m_secondaryWorldFramebuffer->m_depthStencilTarget;
    //The first one was shared with the primary framebuffer
    delete m_secondaryWorldFramebuffer->m_colorTargets[1];
    Framebuffer::FramebufferDelete(m_primaryWorldFramebuffer);
    Framebuffer::FramebufferDelete(m_secondaryWorldFramebuffer);
}

//-----------------------------------------------------------------------------------
void TheGame::Update(float deltaTime)
{
    if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::TILDE))
    {
        Console::instance->ActivateConsole();
    }
    if (Console::instance->IsActive())
    {
        return; //Don't do anything involving game updates.
    }
    if (InputSystem::instance->WasKeyJustPressed('U'))
    {
        SwapWorlds();
    }
    if (InputSystem::instance->WasKeyJustPressed('B'))
    {
        g_renderDebug = !g_renderDebug;
    }
    m_player->Update(deltaTime);
    for (World* world : m_worlds)
    {
        world->Update(deltaTime);
    }
    UpdateDebug();
    std::regex chunkFile("-?[0-9]+,-?[0-9]+.*\\.chunk");
}

//-----------------------------------------------------------------------------------
void TheGame::Render() const
{
    ENSURE_NO_MATRIX_STACK_SIDE_EFFECTS(Renderer::instance->m_viewStack);
    ENSURE_NO_MATRIX_STACK_SIDE_EFFECTS(Renderer::instance->m_projStack);
    //3D RENDERING

    //Renderer::instance->BindFramebuffer(m_secondaryWorldFramebuffer);
    //Renderer::instance->ClearScreen(m_worlds[m_alternateRenderedWorldID]->m_skyColor);
    Renderer::instance->BindFramebuffer(m_primaryWorldFramebuffer);
    //Renderer::instance->ClearScreen(RGBA::WHITE);
    m_primaryWorldFramebuffer->ClearColorBuffer(0, m_worlds[m_currentlyRenderedWorldID]->m_skyColor);
    m_primaryWorldFramebuffer->ClearColorBuffer(1, RGBA::WHITE);
    Renderer::instance->ClearDepth(1.0f);
    Begin3DPerspective();
    m_playerCamera->UpdateViewFromCamera();

    Renderer::instance->EnableFaceCulling(true);
    m_blockMaterial->SetIntUniform("gPassNumber", 0);
    m_blockMaterial->SetEmissiveTexture(Texture::CreateOrGetTexture("Data/Images/initialPortalTexture.png"));
    m_blockMaterial->SetVec4Uniform("gColor", m_worlds[m_alternateRenderedWorldID]->m_skyColor.ToVec4());
    m_worlds[m_currentlyRenderedWorldID]->Render();

    //Second Pass
    Renderer::instance->BindFramebuffer(m_secondaryWorldFramebuffer);
    Renderer::instance->ClearDepth(1.0f);
    //m_secondaryWorldFramebuffer->ClearColorBuffer(0, m_worlds[m_alternateRenderedWorldID]->m_skyColor);
    m_blockMaterial->SetIntUniform("gPassNumber", 1);
    m_blockMaterial->SetEmissiveTexture(m_primaryWorldFramebuffer->m_colorTargets[1]);
    m_blockMaterial->SetNoiseTexture(m_primaryWorldFramebuffer->m_depthStencilTarget);
    m_blockMaterial->SetVec4Uniform("gColor", m_worlds[m_currentlyRenderedWorldID]->m_skyColor.ToVec4());
    m_worlds[m_alternateRenderedWorldID]->Render();
    Renderer::instance->EnableFaceCulling(false);
    Renderer::instance->BindFramebuffer(nullptr);
    Renderer::instance->FrameBufferCopyToBack(m_primaryWorldFramebuffer);
    Renderer::instance->ClearDepth(1.0f);
    m_player->Render();
    if (g_renderDebug)
    {
        RenderAxisLines();
    }
    m_playerCamera->ExitViewFromCamera();
    End3DPerspective();
    //2D RENDERING
    RenderUI();
    if (g_renderDebug)
    {
        Renderer::instance->ClearDepth(1.0f);
        DebugRender();
    }
}

//-----------------------------------------------------------------------------------
void TheGame::DebugRender() const
{
    Renderer::instance->BeginOrtho(Vector2(0.0f, 0.0f), Vector2(1600, 900));
    MeshBuilder builder;
    builder.AddTexturedAABB(AABB2(Vector2::ZERO, Vector2(160, 90)), Vector2::ZERO, Vector2::ONE, RGBA::WHITE);
    Material* debugMaterial = new Material(new ShaderProgram("Data/Shaders/fixedVertexFormat.vert", "Data/Shaders/depthBufferDebug.frag"),
        RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::RENDER_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND));
    MeshRenderer* renderer = new MeshRenderer(new Mesh(), debugMaterial);
    builder.CopyToMesh(renderer->m_mesh, &Vertex_PCUTB::Copy, sizeof(Vertex_PCUTB), &Vertex_PCUTB::BindMeshToVAO);
    debugMaterial->SetDiffuseTexture(m_primaryWorldFramebuffer->m_colorTargets[0]);
    renderer->Render();
    renderer->SetPosition(Vector3(160, 0, 0));
    debugMaterial->SetDiffuseTexture(m_primaryWorldFramebuffer->m_colorTargets[1]);
    renderer->Render();
    renderer->SetPosition(Vector3(320, 0, 0));
    debugMaterial->SetDiffuseTexture(m_secondaryWorldFramebuffer->m_colorTargets[1]);
    renderer->Render();
    renderer->SetPosition(Vector3(480, 0, 0));
    debugMaterial->SetDiffuseTexture(m_primaryWorldFramebuffer->m_depthStencilTarget);
    renderer->Render();
    renderer->SetPosition(Vector3(640, 0, 0));
    debugMaterial->SetDiffuseTexture(m_secondaryWorldFramebuffer->m_depthStencilTarget);
    renderer->Render();
    delete debugMaterial->m_shaderProgram;
    delete debugMaterial;
    delete renderer->m_mesh;
    delete renderer;
    Renderer::instance->EndOrtho();
}

//-----------------------------------------------------------------------------------
void TheGame::RenderUI() const
{
    Renderer::instance->BeginOrtho(Vector2(0.0f, 0.0f), Vector2(1600, 900));
    if (g_renderDebug)
    {
        RenderDebugText();
    }
    RenderCrosshair();
    RenderInventory();
    Renderer::instance->EndOrtho();
}

//-----------------------------------------------------------------------------------
void TheGame::RenderDebugText() const
{
    Renderer::instance->EnableDepthTest(false);
    Vector3 camPos = m_playerCamera->m_position;
    EulerAngles camAngle = m_playerCamera->m_orientation;
    std::string cameraPosition = Stringf("Camera Position: (%f, %f, %f)", camPos.x, camPos.y, camPos.z);
    std::string cameraOrientation = Stringf("Camera Orientation: (%f, %f, %f)", camAngle.rollDegreesAboutX, camAngle.pitchDegreesAboutY, camAngle.yawDegreesAboutZ);
    int numActiveChunks = 0;
    for (World* world : m_worlds)
    {
        numActiveChunks += world->GetNumActiveChunks();
    }
    std::string activeChunks = Stringf("Active Chunks: %i", numActiveChunks);
    std::string physicsMode;
    switch (m_player->m_physicsMode)
    {
    case Player::PhysicsMode::NOCLIP:
        physicsMode = "Mode: NoClip";
        break;
    case Player::PhysicsMode::FLYING:
        physicsMode = "Mode: Flying";
        break;
    case Player::PhysicsMode::WALKING:
        physicsMode = "Mode: Normal";
        break;
    case Player::PhysicsMode::NUM_PHYSICS_MODES:
        physicsMode = "Mode: INVALID MODE";
        break;
    default:
        physicsMode = "Mode: INVALID MODE";
        break;
    }
    TimingInfo frameProfilingInfo = g_profilingResults[g_frameTimeProfiling];
    //Multiply by 1000 to put into milliseconds.
    std::string frameProfiling = Stringf("Frame Times =  Avg: %.02f ms, Max: %.02f ms, Last: %.02f ms", frameProfilingInfo.m_averageSample * 1000.0, frameProfilingInfo.m_maxSample * 1000.0, frameProfilingInfo.m_lastSample * 1000.0);

    TimingInfo loadProfilingInfo = g_profilingResults[g_loadingProfiling];
    //Multiply by 1000 to put into milliseconds.
    std::string loadProfiling = Stringf("Load Times =  Avg: %.02f ms, Max: %.02f ms, Last: %.02f ms", loadProfilingInfo.m_averageSample * 1000.0, loadProfilingInfo.m_maxSample * 1000.0, loadProfilingInfo.m_lastSample * 1000.0);

    TimingInfo saveProfilingInfo = g_profilingResults[g_savingProfiling];
    //Multiply by 1000 to put into milliseconds.
    std::string saveProfiling = Stringf("Save Times =  Avg: %.02f ms, Max: %.02f ms, Last: %.02f ms", saveProfilingInfo.m_averageSample * 1000.0, saveProfilingInfo.m_maxSample * 1000.0, saveProfilingInfo.m_lastSample * 1000.0);

    TimingInfo genProfilingInfo = g_profilingResults[g_generationProfiling];
    //Multiply by 1000 to put into milliseconds.
    std::string genProfiling = Stringf("Generation Times =  Avg: %.02f ms, Max: %.02f ms, Last: %.02f ms", genProfilingInfo.m_averageSample * 1000.0, genProfilingInfo.m_maxSample * 1000.0, genProfilingInfo.m_lastSample * 1000.0);

    TimingInfo vaProfilingInfo = g_profilingResults[g_vaBuildingProfiling];
    //Multiply by 1000 to put into milliseconds.
    std::string vaProfiling = Stringf("VA Times =  Avg: %.02f ms, Max: %.02f ms, Last: %.02f ms", vaProfilingInfo.m_averageSample * 1000.0, vaProfilingInfo.m_maxSample * 1000.0, vaProfilingInfo.m_lastSample * 1000.0);

    TimingInfo tempProfilingInfo = g_profilingResults[g_temporaryProfiling];
    //Multiply by 1000 to put into milliseconds.
    std::string tempProfiling = Stringf("Temporary Profiling =  Avg: %.02f ms, Max: %.02f ms, Last: %.02f ms", tempProfilingInfo.m_averageSample * 1000.0, tempProfilingInfo.m_maxSample * 1000.0, tempProfilingInfo.m_lastSample * 1000.0);

    TimingInfo renderProfilingInfo = g_profilingResults[g_renderProfiling];
    //Multiply by 1000 to put into milliseconds.
    std::string renderProfiling = Stringf("Render Profiling =  Avg: %.02f ms, Max: %.02f ms, Last: %.02f ms", renderProfilingInfo.m_averageSample * 1000.0, renderProfilingInfo.m_maxSample * 1000.0, renderProfilingInfo.m_lastSample * 1000.0);

    TimingInfo updateProfilingInfo = g_profilingResults[g_updateProfiling];
    //Multiply by 1000 to put into milliseconds.
    std::string updateProfiling = Stringf("Update Profiling =  Avg: %.02f ms, Max: %.02f ms, Last: %.02f ms", updateProfilingInfo.m_averageSample * 1000.0, updateProfilingInfo.m_maxSample * 1000.0, updateProfilingInfo.m_lastSample * 1000.0);

    const float FontSize = 20.0f;
    const float FontSquishRatio = 0.65f;
    const float FontWidth = FontSquishRatio * FontSize;
    const float TopLineY = 900.0f - FontSize;
    const float TopRight = TheApp::instance->GetWindowWidth();
    int lineNumber = 0;
    Renderer::instance->DrawText2D(Vector2(0.0f, TopLineY - (FontSize * lineNumber++)), cameraPosition, FontWidth, FontSize, RGBA::WHITE, true);
    Renderer::instance->DrawText2D(Vector2(0.0f, TopLineY - (FontSize * lineNumber++)), cameraOrientation, FontWidth, FontSize, RGBA::WHITE, true);
    Renderer::instance->DrawText2D(Vector2(0.0f, TopLineY - (FontSize * lineNumber++)), activeChunks, FontWidth, FontSize, RGBA::GREEN, true);
    lineNumber++;
    Renderer::instance->DrawText2D(Vector2(0.0f, TopLineY - (FontSize * lineNumber++)), genProfiling, FontWidth, FontSize, RGBA::ORANGE, true);
    Renderer::instance->DrawText2D(Vector2(0.0f, TopLineY - (FontSize * lineNumber++)), loadProfiling, FontWidth, FontSize, RGBA::RED, true);
    Renderer::instance->DrawText2D(Vector2(0.0f, TopLineY - (FontSize * lineNumber++)), saveProfiling, FontWidth, FontSize, RGBA::BLUE, true);
    Renderer::instance->DrawText2D(Vector2(0.0f, TopLineY - (FontSize * lineNumber++)), vaProfiling, FontWidth, FontSize, RGBA::GREEN, true);
    Renderer::instance->DrawText2D(Vector2(0.0f, TopLineY - (FontSize * lineNumber++)), tempProfiling, FontWidth, FontSize, RGBA::MAGENTA, true);
    lineNumber++;
    Renderer::instance->DrawText2D(Vector2(0.0f, TopLineY - (FontSize * lineNumber++)), updateProfiling, FontWidth, FontSize, RGBA::CHOCOLATE, true);
    Renderer::instance->DrawText2D(Vector2(0.0f, TopLineY - (FontSize * lineNumber++)), renderProfiling, FontWidth, FontSize, RGBA::FOREST_GREEN, true);
    Renderer::instance->DrawText2D(Vector2(0.0f, TopLineY - (FontSize * lineNumber++)), frameProfiling, FontWidth, FontSize, RGBA::YELLOW, true);
    Renderer::instance->DrawText2D(Vector2(TopRight - (FontWidth * physicsMode.length()), TopLineY), physicsMode, FontWidth, FontSize, RGBA::BLACK);
}

//-----------------------------------------------------------------------------------
void TheGame::Begin3DPerspective() const
{
    const float aspect = 16.f / 9.f;
    const float nearDist = 0.1f;
    const float farDist = 1000.0f;
    const float fovY = 50.0f;
    Renderer::instance->BeginPerspective(fovY, aspect, nearDist, farDist);
    //Put Z up.
    Renderer::instance->RotateView(-90.0f, Vector3::UNIT_X);
    //Put X forward.
    Renderer::instance->RotateView(90.0f, Vector3::UNIT_Z);
    Renderer::instance->EnableDepthTest(true);
}

//-----------------------------------------------------------------------------------
void TheGame::End3DPerspective() const
{
    //Put X back
    Renderer::instance->PopView();
    //Put Z back
    Renderer::instance->PopView();
    //Reset Perspective
    Renderer::instance->EndPerspective();
}

//-----------------------------------------------------------------------------------
void TheGame::RenderAxisLines() const
{
    const float axisLineLength = 100.0f;
    Renderer::instance->EnableDepthTest(true);

    Renderer::instance->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(axisLineLength, 0.0f, 0.0f), RGBA::RED, 3.0f);
    Renderer::instance->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, axisLineLength, 0.0f), RGBA::GREEN, 3.0f);
    Renderer::instance->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, axisLineLength), RGBA::BLUE, 3.0f);

    Renderer::instance->EnableDepthTest(false);

    Renderer::instance->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(axisLineLength, 0.0f, 0.0f), RGBA::RED, 1.0f);
    Renderer::instance->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, axisLineLength, 0.0f), RGBA::GREEN, 1.0f);
    Renderer::instance->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, axisLineLength), RGBA::BLUE, 1.0f);

    Renderer::instance->EnableDepthTest(true);
}

//-----------------------------------------------------------------------------------
void TheGame::RenderCrosshair() const
{
    Vector2 screenCenter = Vector2(TheApp::instance->GetWindowWidth() / 2.0f, TheApp::instance->GetWindowHeight() / 2.0f);
    float crosshairSize = 20.0f;
    float crosshairThickness = 4.0f;

    Renderer::instance->EnableInvertedBlending();
    Renderer::instance->DrawLine(screenCenter + (Vector2::UNIT_X * crosshairSize), screenCenter - (Vector2::UNIT_X * crosshairSize), RGBA::WHITE, crosshairThickness);
    Renderer::instance->DrawLine(screenCenter + (Vector2::UNIT_Y * crosshairSize), screenCenter - (Vector2::UNIT_Y * crosshairSize), RGBA::WHITE, crosshairThickness);
    Renderer::instance->EnableAlphaBlending();
}

//-----------------------------------------------------------------------------------
void TheGame::RenderInventory() const
{
    MeshBuilder builder;
    float blockSize = 50.0f;
    Vector2 offset = Vector2(TheApp::instance->GetWindowWidth() / 4.0f, 0.0f);

    builder.Begin();
    for (uchar i = 0; i < Player::NUM_INVENTORY_SLOTS; i++)
    {
        uchar index = static_cast<uchar>(m_player->m_firstBlockInInventory) + i;
        if (index >= BlockType::NUM_BLOCKS)
        {
            index = BlockType::AIR + (index - NUM_BLOCKS);
        }
        else if (index < BlockType::AIR)
        {
            index = BlockType::NUM_BLOCKS - i;
        }
        AABB2 texture = BlockDefinition::GetDefinition(index)->GetSideIndex();
        blockSize = 50.0f;
        if (m_player->m_highlightedUIBlock == i)
        {
            blockSize = 100.0f;
        }
        builder.AddTexturedAABB(AABB2(Vector2(offset.x, offset.y), Vector2(offset.x + blockSize, offset.y + blockSize)), texture.mins, texture.maxs, RGBA::WHITE);
        offset.x += blockSize + 25.0f;
    }
    builder.End();
    Mesh* blockMesh = new Mesh();
    builder.CopyToMesh(blockMesh, &Vertex_PCUTB::Copy, sizeof(Vertex_PCUTB), &Vertex_PCUTB::BindMeshToVAO);
    MeshRenderer* blockSelection = new MeshRenderer(blockMesh, m_blockMaterialWithoutPortals);
    blockSelection->Render();
    delete blockMesh;
    delete blockSelection;
}

//-----------------------------------------------------------------------------------
void TheGame::SwapWorlds()
{
    m_currentlyRenderedWorldID = m_alternateRenderedWorldID;
    m_alternateRenderedWorldID = m_currentlyRenderedWorldID == 0 ? 1 : 0;
    m_player->m_world = m_worlds[m_currentlyRenderedWorldID];
    AudioSystem::instance->PlaySound(m_worldSwapSFX);
}