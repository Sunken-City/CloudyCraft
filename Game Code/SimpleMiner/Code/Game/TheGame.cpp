#include "Game/TheGame.hpp"
#include "Game/TheApp.hpp"
#include "Game/Camera3D.hpp"
#include "Game/World.hpp"
#include "Game/BlockDefinition.h"
#include "Game/Player.hpp"
#include "Game/Generator.hpp"
#include "Engine/Renderer/TheRenderer.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Vertex.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Renderer/AABB3.hpp"
#include "Engine/Input/XInputController.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/Console.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <thread>

TheGame* TheGame::instance = nullptr;

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
{
	m_worlds.push_back(new World(0, RGBA(0xDDEEFF00), RGBA(0x4DC9FF00), new EarthGenerator()));			//BlueSky 0x4DC9FFFF     Vaporwave 0xFF819CFF
	m_worlds.push_back(new World(1, RGBA(0xFDDA0E00), RGBA(0xC5540900), new SkylandsGenerator()));
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
}

//-----------------------------------------------------------------------------------
TheGame::~TheGame()
{
	for (World* world : m_worlds)
	{
		delete world;
	}
	delete m_player;
	delete m_blockSheet;
}
float degrees = 0.f;

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
	if (InputSystem::instance->WasKeyJustPressed('U'))
	{
		m_currentlyRenderedWorldID = m_currentlyRenderedWorldID == 0 ? 1 : 0;
		m_player->m_world = m_worlds[m_currentlyRenderedWorldID];
	}
}

//-----------------------------------------------------------------------------------
void TheGame::Render() const
{
	//3D RENDERING
	TheRenderer::instance->ClearScreen(m_worlds[m_currentlyRenderedWorldID]->m_skyColor);
	SetUp3DPerspective();
	m_playerCamera->UpdateViewFromCamera();
	TheRenderer::instance->EnableFaceCulling(true);
	m_worlds[m_currentlyRenderedWorldID]->Render();
	TheRenderer::instance->EnableFaceCulling(false);
	m_player->Render();
	if (g_renderDebug)
	{
		RenderAxisLines();
	}
	//2D RENDERING
	RenderUI();
}

//-----------------------------------------------------------------------------------
void TheGame::RenderUI() const
{
	TheRenderer::instance->SetOrtho(Vector2(0.0f, 0.0f), Vector2(1600, 900));
	if (g_renderDebug)
	{
		RenderDebugText();
	}
	RenderCrosshair();
	RenderInventory();
}

//-----------------------------------------------------------------------------------
void TheGame::RenderDebugText() const
{
	TheRenderer::instance->EnableDepthTest(false);
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
	TimingInfo frameProfilingInfo = profilingResults[g_frameTimeProfiling];
	//Multiply by 1000 to put into milliseconds.
	std::string frameProfiling = Stringf("Frame Times =  Avg: %.02f ms, Max: %.02f ms, Last: %.02f ms", frameProfilingInfo.m_averageSample * 1000.0, frameProfilingInfo.m_maxSample * 1000.0, frameProfilingInfo.m_lastSample * 1000.0);

	TimingInfo loadProfilingInfo = profilingResults[g_loadingProfiling];
	//Multiply by 1000 to put into milliseconds.
	std::string loadProfiling = Stringf("Load Times =  Avg: %.02f ms, Max: %.02f ms, Last: %.02f ms", loadProfilingInfo.m_averageSample * 1000.0, loadProfilingInfo.m_maxSample * 1000.0, loadProfilingInfo.m_lastSample * 1000.0);

	TimingInfo saveProfilingInfo = profilingResults[g_savingProfiling];
	//Multiply by 1000 to put into milliseconds.
	std::string saveProfiling = Stringf("Save Times =  Avg: %.02f ms, Max: %.02f ms, Last: %.02f ms", saveProfilingInfo.m_averageSample * 1000.0, saveProfilingInfo.m_maxSample * 1000.0, saveProfilingInfo.m_lastSample * 1000.0);

	TimingInfo genProfilingInfo = profilingResults[g_generationProfiling];
	//Multiply by 1000 to put into milliseconds.
	std::string genProfiling = Stringf("Generation Times =  Avg: %.02f ms, Max: %.02f ms, Last: %.02f ms", genProfilingInfo.m_averageSample * 1000.0, genProfilingInfo.m_maxSample * 1000.0, genProfilingInfo.m_lastSample * 1000.0);

	TimingInfo vaProfilingInfo = profilingResults[g_vaBuildingProfiling];
	//Multiply by 1000 to put into milliseconds.
	std::string vaProfiling = Stringf("VA Times =  Avg: %.02f ms, Max: %.02f ms, Last: %.02f ms", vaProfilingInfo.m_averageSample * 1000.0, vaProfilingInfo.m_maxSample * 1000.0, vaProfilingInfo.m_lastSample * 1000.0);

	TimingInfo tempProfilingInfo = profilingResults[g_temporaryProfiling];
	//Multiply by 1000 to put into milliseconds.
	std::string tempProfiling = Stringf("Temporary Profiling =  Avg: %.02f ms, Max: %.02f ms, Last: %.02f ms", tempProfilingInfo.m_averageSample * 1000.0, tempProfilingInfo.m_maxSample * 1000.0, tempProfilingInfo.m_lastSample * 1000.0);

	TimingInfo renderProfilingInfo = profilingResults[g_renderProfiling];
	//Multiply by 1000 to put into milliseconds.
	std::string renderProfiling = Stringf("Render Profiling =  Avg: %.02f ms, Max: %.02f ms, Last: %.02f ms", renderProfilingInfo.m_averageSample * 1000.0, renderProfilingInfo.m_maxSample * 1000.0, renderProfilingInfo.m_lastSample * 1000.0);

	TimingInfo updateProfilingInfo = profilingResults[g_updateProfiling];
	//Multiply by 1000 to put into milliseconds.
	std::string updateProfiling = Stringf("Update Profiling =  Avg: %.02f ms, Max: %.02f ms, Last: %.02f ms", updateProfilingInfo.m_averageSample * 1000.0, updateProfilingInfo.m_maxSample * 1000.0, updateProfilingInfo.m_lastSample * 1000.0);

	const float FontSize = 20.0f;
	const float FontSquishRatio = 0.65f;
	const float FontWidth = FontSquishRatio * FontSize;
	const float TopLineY = 900.0f - FontSize;
	const float TopRight = TheApp::instance->GetWindowWidth();
	int lineNumber = 0;
	TheRenderer::instance->DrawText2D(Vector2(0.0f, TopLineY - (FontSize * lineNumber++)), cameraPosition, FontWidth, FontSize, RGBA::WHITE, true);
	TheRenderer::instance->DrawText2D(Vector2(0.0f, TopLineY - (FontSize * lineNumber++)), cameraOrientation, FontWidth, FontSize, RGBA::WHITE, true);
	TheRenderer::instance->DrawText2D(Vector2(0.0f, TopLineY - (FontSize * lineNumber++)), activeChunks, FontWidth, FontSize, RGBA::GREEN, true);
	lineNumber++;
	TheRenderer::instance->DrawText2D(Vector2(0.0f, TopLineY - (FontSize * lineNumber++)), genProfiling, FontWidth, FontSize, RGBA::ORANGE, true);
	TheRenderer::instance->DrawText2D(Vector2(0.0f, TopLineY - (FontSize * lineNumber++)), loadProfiling, FontWidth, FontSize, RGBA::RED, true);
	TheRenderer::instance->DrawText2D(Vector2(0.0f, TopLineY - (FontSize * lineNumber++)), saveProfiling, FontWidth, FontSize, RGBA::BLUE, true);
	TheRenderer::instance->DrawText2D(Vector2(0.0f, TopLineY - (FontSize * lineNumber++)), vaProfiling, FontWidth, FontSize, RGBA::GREEN, true);
	TheRenderer::instance->DrawText2D(Vector2(0.0f, TopLineY - (FontSize * lineNumber++)), tempProfiling, FontWidth, FontSize, RGBA::MAGENTA, true);
	lineNumber++;
	TheRenderer::instance->DrawText2D(Vector2(0.0f, TopLineY - (FontSize * lineNumber++)), updateProfiling, FontWidth, FontSize, RGBA::CHOCOLATE, true);
	TheRenderer::instance->DrawText2D(Vector2(0.0f, TopLineY - (FontSize * lineNumber++)), renderProfiling, FontWidth, FontSize, RGBA::FOREST_GREEN, true);
	TheRenderer::instance->DrawText2D(Vector2(0.0f, TopLineY - (FontSize * lineNumber++)), frameProfiling, FontWidth, FontSize, RGBA::YELLOW, true);
	TheRenderer::instance->DrawText2D(Vector2(TopRight - (FontWidth * physicsMode.length()), TopLineY), physicsMode, FontWidth, FontSize, RGBA::BLACK);
}

//-----------------------------------------------------------------------------------
void TheGame::SetUp3DPerspective() const
{
	const float aspect = 16.f / 9.f;
	const float nearDist = 0.1f;
	const float farDist = 1000.0f;
	const float fovY = 50.0f;
	TheRenderer::instance->SetPerspective(fovY, aspect, nearDist, farDist);

	//Put Z up.
	TheRenderer::instance->Rotate(-90.0f, 1.f, 0.f, 0.f);

	//Put X forward.
	TheRenderer::instance->Rotate(90.0f, 0.f, 0.f, 1.f);

	TheRenderer::instance->EnableDepthTest(true);
}

//-----------------------------------------------------------------------------------
void TheGame::RenderAxisLines() const
{
	const float axisLineLength = 100.0f;
	TheRenderer::instance->EnableDepthTest(true);

	TheRenderer::instance->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(axisLineLength, 0.0f, 0.0f), RGBA::RED, 3.0f);
	TheRenderer::instance->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, axisLineLength, 0.0f), RGBA::GREEN, 3.0f);
	TheRenderer::instance->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, axisLineLength), RGBA::BLUE, 3.0f);

	TheRenderer::instance->EnableDepthTest(false);

	TheRenderer::instance->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(axisLineLength, 0.0f, 0.0f), RGBA::RED, 1.0f);
	TheRenderer::instance->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, axisLineLength, 0.0f), RGBA::GREEN, 1.0f);
	TheRenderer::instance->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, axisLineLength), RGBA::BLUE, 1.0f);

	TheRenderer::instance->EnableDepthTest(true);
}

//-----------------------------------------------------------------------------------
void TheGame::RenderCrosshair() const
{
	Vector2 screenCenter = Vector2(TheApp::instance->GetWindowWidth() / 2.0f, TheApp::instance->GetWindowHeight() / 2.0f);
	float crosshairSize = 20.0f;
	float crosshairThickness = 4.0f;

	TheRenderer::instance->EnableInvertedBlending();
	TheRenderer::instance->DrawLine(screenCenter + (Vector2::UNIT_X * crosshairSize), screenCenter - (Vector2::UNIT_X * crosshairSize), RGBA::WHITE, crosshairThickness);
	TheRenderer::instance->DrawLine(screenCenter + (Vector2::UNIT_Y * crosshairSize), screenCenter - (Vector2::UNIT_Y * crosshairSize), RGBA::WHITE, crosshairThickness);
	TheRenderer::instance->EnableAlphaBlending();
}

//-----------------------------------------------------------------------------------
void TheGame::RenderInventory() const
{
	TheRenderer::instance->EnableDepthTest(false);
	float blockSize = 50.0f;
	std::vector<Vertex_PCT> vertexes;
	Vertex_PCT vertex;
	vertex.color = RGBA::WHITE;

	Vector2 offset = Vector2(TheApp::instance->GetWindowWidth() / 4.0f, 0.0f);
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
		vertex.texCoords = texture.mins;
		vertex.pos = Vector3(offset.x, offset.y, 0.0f);
		vertexes.push_back(vertex);
		vertex.texCoords = Vector2(texture.mins.x, texture.maxs.y);
		vertex.pos = Vector3(offset.x, offset.y + blockSize, 0.0f);
		vertexes.push_back(vertex);
		vertex.texCoords = texture.maxs;
		vertex.pos = Vector3(offset.x + blockSize, offset.y + blockSize, 0.0f);
		vertexes.push_back(vertex);
		vertex.texCoords = Vector2(texture.maxs.x, texture.mins.y);
		vertex.pos = Vector3(offset.x + blockSize, offset.y, 0.0f);
		vertexes.push_back(vertex);
		offset.x += blockSize + 25.0f;
	}
	TheRenderer::instance->DrawVertexArray(vertexes.data(), vertexes.size(), TheRenderer::QUADS, m_blockSheet->GetTexture());
	TheRenderer::instance->EnableDepthTest(true);
}
