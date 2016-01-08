#include "Game/TheGame.hpp"
#include "Game/TheApp.hpp"
#include "Engine/Renderer/TheRenderer.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Input/XInputController.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Game/Camera3D.hpp"
#include "Game/World.hpp"
#include "Game/BlockDefinition.h"
#include "Game/Player.hpp"
#include "Engine/Renderer/Vertex.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Renderer/AABB3.hpp"
#include <thread>

TheGame* TheGame::instance = nullptr;

const RGBA TheGame::SKY_COLOR = RGBA(0x4DC9FFFF);//BlueSky 0x4DC9FFFF     Vaporwave 0xFF819CFF

TheGame::TheGame()
: m_blockSheet(new SpriteSheet("Data/Images/SimpleMinerAtlas.png", 16, 16))
, m_world(new World())
{
	//Why does this have to be here? I had it in initializer list, but caused race condition.
	m_player = new Player(m_world);
	m_playerCamera = &(m_player->m_camera);
	m_world->CreateRenderingOffsetList(m_player);
}

TheGame::~TheGame()
{
	delete m_world;
	delete m_player;
	delete m_blockSheet;
}
float degrees = 0.f;

void TheGame::Update(float deltaTime)
{
	m_player->Update(deltaTime);
	m_world->Update(deltaTime);
}

void TheGame::Render() const
{
	TheRenderer::instance->ClearScreen(SKY_COLOR);
	SetUp3DPerspective();
	m_playerCamera->UpdateViewFromCamera();
	TheRenderer::instance->EnableFaceCulling(true);
	m_world->Render();
	TheRenderer::instance->EnableFaceCulling(false);
	m_player->Render();
	RenderAxisLines(); 
	RenderUI();
}

void TheGame::RenderUI() const
{
	TheRenderer::instance->SetOrtho(Vector2(0.0f, 0.0f), Vector2(1600, 900));
	RenderDebugText();
	RenderCrosshair();
	RenderInventory();
}

void TheGame::RenderDebugText() const
{
	TheRenderer::instance->EnableDepthTest(false);
	Vector3 camPos = m_playerCamera->m_position;
	EulerAngles camAngle = m_playerCamera->m_orientation;
	std::string cameraPosition = Stringf("Camera Position: (%f, %f, %f)", camPos.x, camPos.y, camPos.z);
	std::string cameraOrientation = Stringf("Camera Orientation: (%f, %f, %f)", camAngle.rollDegreesAboutX, camAngle.pitchDegreesAboutY, camAngle.yawDegreesAboutZ);
	std::string activeChunks = Stringf("Active Chunks: %i", m_world->GetNumActiveChunks());
	std::string physicsMode;
	switch (m_player->m_physicsMode)
	{
	case Player::PhysicsMode::NOCLIP:
		physicsMode = "Mode: NoClip";
		break;
	case Player::PhysicsMode::FLYING:
		physicsMode = "Mode: Flying";
		break;
	case Player::PhysicsMode::NORMAL:
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
	std::string frameProfiling = Stringf("Frame Times =  Avg: %f ms, Max: %f ms, Last: %f ms", frameProfilingInfo.m_averageSample * 1000.0, frameProfilingInfo.m_maxSample * 1000.0, frameProfilingInfo.m_lastSample * 1000.0);

	TimingInfo loadProfilingInfo = profilingResults[g_loadingProfiling];
	//Multiply by 1000 to put into milliseconds.
	std::string loadProfiling = Stringf("Load Times =  Avg: %f ms, Max: %f ms, Last: %f ms", loadProfilingInfo.m_averageSample * 1000.0, loadProfilingInfo.m_maxSample * 1000.0, loadProfilingInfo.m_lastSample * 1000.0);

	TimingInfo saveProfilingInfo = profilingResults[g_savingProfiling];
	//Multiply by 1000 to put into milliseconds.
	std::string saveProfiling = Stringf("Save Times =  Avg: %f ms, Max: %f ms, Last: %f ms", saveProfilingInfo.m_averageSample * 1000.0, saveProfilingInfo.m_maxSample * 1000.0, saveProfilingInfo.m_lastSample * 1000.0);

	TimingInfo genProfilingInfo = profilingResults[g_generationProfiling];
	//Multiply by 1000 to put into milliseconds.
	std::string genProfiling = Stringf("Generation Times =  Avg: %f ms, Max: %f ms, Last: %f ms", genProfilingInfo.m_averageSample * 1000.0, genProfilingInfo.m_maxSample * 1000.0, genProfilingInfo.m_lastSample * 1000.0);

	TimingInfo vaProfilingInfo = profilingResults[g_vaBuildingProfiling];
	//Multiply by 1000 to put into milliseconds.
	std::string vaProfiling = Stringf("VA Times =  Avg: %f ms, Max: %f ms, Last: %f ms", vaProfilingInfo.m_averageSample * 1000.0, vaProfilingInfo.m_maxSample * 1000.0, vaProfilingInfo.m_lastSample * 1000.0);

	TimingInfo tempProfilingInfo = profilingResults[g_temporaryProfiling];
	//Multiply by 1000 to put into milliseconds.
	std::string tempProfiling = Stringf("Temporary Profiling =  Avg: %f ms, Max: %f ms, Last: %f ms", tempProfilingInfo.m_averageSample * 1000.0, tempProfilingInfo.m_maxSample * 1000.0, tempProfilingInfo.m_lastSample * 1000.0);

	const float FontSize = 20.0f;
	const float FontSquishRatio = 0.65f;
	const float FontWidth = FontSquishRatio * FontSize;
	const float TopLineY = 900.0f - FontSize;
	const float SecondLineY = TopLineY - FontSize;
	const float ThirdLineY = SecondLineY - FontSize;
	const float FourthLineY = ThirdLineY - FontSize;
	const float FifthLineY = FourthLineY - FontSize;
	const float SixthLineY = FifthLineY - FontSize;
	const float SeventhLineY = SixthLineY - FontSize;
	const float EighthLineY = SeventhLineY - FontSize;
	const float NinethLineY = EighthLineY - FontSize;
	const float TopRight = TheApp::instance->GetWindowWidth();
	TheRenderer::instance->DrawText2D(Vector2(0.0f, TopLineY), cameraPosition, FontWidth, FontSize, RGBA::WHITE, true);
	TheRenderer::instance->DrawText2D(Vector2(0.0f, SecondLineY), cameraOrientation, FontWidth, FontSize, RGBA::WHITE, true);
	TheRenderer::instance->DrawText2D(Vector2(0.0f, ThirdLineY), activeChunks, FontWidth, FontSize, RGBA::GREEN, true);
	TheRenderer::instance->DrawText2D(Vector2(0.0f, FourthLineY), genProfiling, FontWidth, FontSize, RGBA::ORANGE, true);
	TheRenderer::instance->DrawText2D(Vector2(0.0f, FifthLineY), loadProfiling, FontWidth, FontSize, RGBA::RED, true);
	TheRenderer::instance->DrawText2D(Vector2(0.0f, SixthLineY), saveProfiling, FontWidth, FontSize, RGBA::BLUE, true);
	TheRenderer::instance->DrawText2D(Vector2(0.0f, SeventhLineY), vaProfiling, FontWidth, FontSize, RGBA::GREEN, true);
	TheRenderer::instance->DrawText2D(Vector2(0.0f, EighthLineY), tempProfiling, FontWidth, FontSize, RGBA::MAGENTA, true);
	TheRenderer::instance->DrawText2D(Vector2(0.0f, NinethLineY), frameProfiling, FontWidth, FontSize, RGBA::YELLOW, true);
	TheRenderer::instance->DrawText2D(Vector2(TopRight - (FontWidth * physicsMode.length()), TopLineY), physicsMode, FontWidth, FontSize, RGBA::BLACK);
}

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

void TheGame::RenderInventory() const
{
	TheRenderer::instance->EnableDepthTest(false);
	float blockSize = 50.0f;
	std::vector<Vertex_PCT> vertexes;
	Vertex_PCT vertex;
	vertex.color = RGBA::WHITE;
	
	int highlightedBlock = m_player->m_highlightedUIBlock;

	Vector2 offset = Vector2(TheApp::instance->GetWindowWidth() / (float)Player::NUM_INVENTORY_SLOTS, 0.0f);
	for (int i = 0; i < Player::NUM_INVENTORY_SLOTS; i++)
	{
		AABB2 texture = BlockDefinition::GetDefinition(m_player->m_firstBlockInInventory + i)->GetSideIndex(); 
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
