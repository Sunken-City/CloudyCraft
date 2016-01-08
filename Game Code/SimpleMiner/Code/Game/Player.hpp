#pragma once
#include "Engine/Renderer/AABB3.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Game/Camera3D.hpp"
#include "Game/GameCommon.hpp"
#include "Game/World.hpp"
#include "Game/BlockDefinition.h"

class Texture;
class SpriteAnim;

class Player
{
public:
	enum class PhysicsMode
	{
		NOCLIP = 0,
		FLYING,
		NORMAL,
		NUM_PHYSICS_MODES
	};

	Player(World* world, PhysicsMode physicsMode = PhysicsMode::NOCLIP);
	~Player();
	void Update(float deltaTime);
	void Render() const;

	void RenderDiggingAnimation() const;

	void RenderFaceHighlightOnRaycastBlock() const;
	void Render2DPlayerTexture() const;
	void RenderPlayerBoundingBox() const;

	RaycastResult3D RaycastForMovementCheck(WorldPosition proposedPosition);
	void LineOfSightRaycast(float deltaTime);
	void UpdateFromKeyboard(float deltaTime);
	void MoveFromKeyboard(float deltaTime);

	void UpdatePhysics(float deltaTime, Vector3 &potentialOffset);

	void SelectBlockFromInventory();
	bool IsOnGround();
	static const float PLAYER_HEIGHT;
	static const float PLAYER_WIDTH;
	static const float HALF_PLAYER_HEIGHT;
	static const float HALF_PLAYER_WIDTH;
	static const float COLLISION_AVOIDANCE_HALF_PLAYER_HEIGHT;
	static const float COLLISION_AVOIDANCE_PLAYER_WIDTH;
	static const float COLLISION_AVOIDANCE_OFFSET;
	static const float EYE_HEIGHT;
	static const int NUM_INVENTORY_SLOTS = 9;
	static const Vector3 EYE_OFFSET;
	static float GRAVITY_CONSTANT;
	static Vector3 ACCELERATION_DUE_TO_GRAVITY;

	Camera3D m_camera;
	PhysicsMode m_physicsMode;
	Vector3 m_position;
	RaycastResult3D m_raycastResult;
	World* m_world;

	uchar m_heldBlock;
	BlockType m_firstBlockInInventory;
	int m_highlightedUIBlock;

private:
	Vector3 m_velocity;
	Vector3 m_acceleration;
	AABB3 m_boundingBox;
	bool m_drawBoundingBox;
	bool m_isDigging;
	float m_breakingBlockTimer;
	BlockInfo m_blockBeingBroken;
	SpriteAnim* m_blockBreakAnimation;
	SoundID m_uiSelectSound;

	//Raycast Debug
	WorldPosition m_raycastImpact;
	WorldPosition m_playerEyes;
	WorldPosition m_target;
	RGBA m_raycastColor;
	Texture* m_texture;
};
