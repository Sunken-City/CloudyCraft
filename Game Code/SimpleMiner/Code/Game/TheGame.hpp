#pragma once
#include <vector>
#include "Engine/Audio/Audio.hpp"

class SpriteSheet;
class RGBA;
class Camera3D;
class World;
class Player;

class TheGame
{
public:
	TheGame();
	~TheGame();
	void Update(float deltaTime);
	void SetUp3DPerspective() const;
	void Render() const;

	void RenderUI() const;
	void RenderDebugText() const;
	void RenderAxisLines() const;
	void RenderCrosshair() const;
	void RenderInventory() const;

	static TheGame* instance;

	SpriteSheet* m_blockSheet;
	Camera3D* m_playerCamera;
	Player* m_player;
	std::vector<World*> m_worlds;
	SoundID m_worldSwapSFX;

private:
	//Ignore warnings for being unable to generate a copy constructor for singleton class.
	TheGame& operator= (const TheGame& other);
	void UpdateDebug();
	int m_currentlyRenderedWorldID;
};
