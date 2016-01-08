#pragma once

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
	void UpdateCamera(float deltaTime);
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
	World* m_world;

private:
	TheGame& operator= (const TheGame& other);
	static const RGBA SKY_COLOR;
};
