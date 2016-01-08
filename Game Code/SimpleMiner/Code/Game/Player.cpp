#include "Game/Player.hpp"
#include "Game/GameCommon.hpp"
#include "Game/World.hpp"
#include "Game/BlockDefinition.h"
#include "Game/Block.hpp"
#include "Engine/Renderer/Face.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteAnim.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

const float Player::PLAYER_HEIGHT = 1.85f;
const float Player::EYE_HEIGHT = 1.62f;
const float Player::PLAYER_WIDTH = 0.6f;
const float Player::HALF_PLAYER_HEIGHT = PLAYER_HEIGHT / 2.0f;
const float Player::HALF_PLAYER_WIDTH = PLAYER_WIDTH / 2.0f;
const float Player::COLLISION_AVOIDANCE_OFFSET = 0.0001f;
const float Player::COLLISION_AVOIDANCE_HALF_PLAYER_HEIGHT = HALF_PLAYER_HEIGHT - COLLISION_AVOIDANCE_OFFSET;
const float Player::COLLISION_AVOIDANCE_PLAYER_WIDTH = HALF_PLAYER_WIDTH - COLLISION_AVOIDANCE_OFFSET;
const Vector3 Player::EYE_OFFSET = Vector3(0.0f, 0.0f, EYE_HEIGHT - HALF_PLAYER_HEIGHT);
float Player::GRAVITY_CONSTANT = 9.81f; 
Vector3 Player::ACCELERATION_DUE_TO_GRAVITY = Vector3(0.0f, 0.0f, -GRAVITY_CONSTANT);

Player::Player(World* world, PhysicsMode physicsMode /*= PhysicsMode::NOCLIP*/)
: m_camera(Camera3D(this))
, m_world(world)
, m_physicsMode(physicsMode)
, m_drawBoundingBox(true)
, m_position(8.f, 8.f, 100.f)
, m_velocity(0.0f, 0.0f, 0.0f)
, m_acceleration(0.0f, 0.0f, 0.0f)
, m_boundingBox(Vector3(-HALF_PLAYER_WIDTH, -HALF_PLAYER_WIDTH, -HALF_PLAYER_HEIGHT), Vector3(HALF_PLAYER_WIDTH, HALF_PLAYER_WIDTH, HALF_PLAYER_HEIGHT))
, m_heldBlock(DIRT)
, m_texture(Texture::CreateOrGetTexture("Data/Images/Pico.png"))
, m_breakingBlockTimer(0.0f)
, m_blockBreakAnimation(new SpriteAnim(*BlockDefinition::m_blockSheet, 1.0f, AnimMode::ONE_SHOT, 0xE0, 0xE9))
, m_isDigging(false)
, m_uiSelectSound(AudioSystem::instance->CreateOrGetSound("Data/SFX/select.wav"))
, m_highlightedUIBlock(0)
, m_firstBlockInInventory(BlockType::STONE)
{

}

Player::~Player()
{
}

void Player::Update(float deltaTime)
{
	UpdateFromKeyboard(deltaTime);
	m_blockBreakAnimation->Update(deltaTime);
	LineOfSightRaycast(deltaTime);

}

void Player::Render() const
{
	if (m_drawBoundingBox)
	{
		RenderPlayerBoundingBox();
	}
	Render2DPlayerTexture();

	//Raycast Debug
	TheRenderer::instance->DrawLine(m_playerEyes, m_target, m_raycastColor, 5.0f);
	TheRenderer::instance->EnableDepthTest(false);
	TheRenderer::instance->DrawLine(m_playerEyes, m_target, m_raycastColor, 1.0f);

	//Highlight face when impacted.
	if (m_raycastResult.didImpact)
	{
		RenderFaceHighlightOnRaycastBlock();
		if (m_isDigging)
		{
			RenderDiggingAnimation();
		}
	}
	DrawDebugPoints(10.0f, true);
}

void Player::RenderDiggingAnimation() const
{
	TheRenderer::instance->EnableDepthTest(true);
	AABB2 texCoords = m_blockBreakAnimation->GetCurrentTexCoords();

	//replace this with a difference in my position and the block's to determine which faces are visible.
	Face west = Block::GetFace(m_raycastResult.impactTileCoords, Vector3Int::UNIT_X);
	Face east = Block::GetFace(m_raycastResult.impactTileCoords, Vector3Int::UNIT_X * -1);
	Face north = Block::GetFace(m_raycastResult.impactTileCoords, Vector3Int::UNIT_Y);
	Face south = Block::GetFace(m_raycastResult.impactTileCoords, Vector3Int::UNIT_Y * -1);
	Face top = Block::GetFace(m_raycastResult.impactTileCoords, Vector3Int::UNIT_Z);
	Face bottom = Block::GetFace(m_raycastResult.impactTileCoords, Vector3Int::UNIT_Z * -1);

	TheRenderer::instance->DrawTexturedFace(west, texCoords.mins, texCoords.maxs, m_blockBreakAnimation->GetTexture(), RGBA(1.0f, 1.0f, 1.0f, 0.2f));
	TheRenderer::instance->DrawTexturedFace(east, texCoords.mins, texCoords.maxs, m_blockBreakAnimation->GetTexture(), RGBA(1.0f, 1.0f, 1.0f, 0.2f));
	TheRenderer::instance->DrawTexturedFace(north, texCoords.mins, texCoords.maxs, m_blockBreakAnimation->GetTexture(), RGBA(1.0f, 1.0f, 1.0f, 0.2f));
	TheRenderer::instance->DrawTexturedFace(south, texCoords.mins, texCoords.maxs, m_blockBreakAnimation->GetTexture(), RGBA(1.0f, 1.0f, 1.0f, 0.2f));
	TheRenderer::instance->DrawTexturedFace(top, texCoords.mins, texCoords.maxs, m_blockBreakAnimation->GetTexture(), RGBA(1.0f, 1.0f, 1.0f, 0.2f));
	TheRenderer::instance->DrawTexturedFace(bottom, texCoords.mins, texCoords.maxs, m_blockBreakAnimation->GetTexture(), RGBA(1.0f, 1.0f, 1.0f, 0.2f));
}

void Player::RenderFaceHighlightOnRaycastBlock() const
{
	Vector3 vertOffset = m_camera.GetForwardXYZ() * -0.01f;
	TheRenderer::instance->SetLineWidth(2.0f);
	TheRenderer::instance->EnableDepthTest(true);
	Face highlightedFace = Block::GetFace(m_raycastResult.impactTileCoords, m_raycastResult.impactSurfaceNormal);
	std::vector<Vertex_PCT> vertexes;
	Vertex_PCT vertex;
	vertex.color = RGBA(1.0f, 1.0f, 1.0f, 1.0f);
	vertex.pos = highlightedFace.verts[0] + vertOffset;
	vertexes.push_back(vertex);
	vertex.pos = highlightedFace.verts[1] + vertOffset;
	vertexes.push_back(vertex);
	vertex.pos = highlightedFace.verts[2] + vertOffset;
	vertexes.push_back(vertex);
	vertex.pos = highlightedFace.verts[3] + vertOffset;
	vertexes.push_back(vertex);
	TheRenderer::instance->DrawVertexArray(vertexes.data(), vertexes.size(), TheRenderer::LINE_LOOP);
}

void Player::Render2DPlayerTexture() const
{
	//Render Player
	Vector3 bottomLeft = m_position - (Vector3::UNIT_X * HALF_PLAYER_WIDTH * 1.5f) - (Vector3::UNIT_Z * HALF_PLAYER_HEIGHT);
	Vector3 bottomRight = m_position + (Vector3::UNIT_X * HALF_PLAYER_WIDTH * 1.5f) - (Vector3::UNIT_Z * HALF_PLAYER_HEIGHT);
	Vector3 topLeft = m_position - (Vector3::UNIT_X * HALF_PLAYER_WIDTH * 1.5f) + (Vector3::UNIT_Z * HALF_PLAYER_HEIGHT);
	Vector3 topRight = m_position + (Vector3::UNIT_X * HALF_PLAYER_WIDTH * 1.5f) + (Vector3::UNIT_Z * HALF_PLAYER_HEIGHT);
	Face playerQuad = Face(topRight, topLeft, bottomLeft, bottomRight);
	TheRenderer::instance->EnableDepthTest(true);
	TheRenderer::instance->DrawTexturedFace(playerQuad, Vector2::ZERO, Vector2::ONE, m_texture, RGBA::WHITE);
}

void Player::RenderPlayerBoundingBox() const
{
	TheRenderer::instance->EnableDepthTest(true);
	TheRenderer::instance->SetLineWidth(4.0f);
	TheRenderer::instance->DrawAABBBoundingBox(m_boundingBox + m_position, RGBA::MAGENTA);
	TheRenderer::instance->EnableDepthTest(false);
	TheRenderer::instance->SetLineWidth(1.0f);
	TheRenderer::instance->DrawAABBBoundingBox(m_boundingBox + m_position, RGBA::MAGENTA);
}

RaycastResult3D Player::RaycastForMovementCheck(WorldPosition proposedPosition)
{
	RaycastResult3D result;
	result.didImpact = false;
	result.impactFraction = 1.0f;
	result.impactPosition = Vector3::ZERO;
	result.impactSurfaceNormal = Vector3Int(0, 0, 0);

	Vector3 bottomBottomLeftStart = m_position		+ Vector3(-COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_HALF_PLAYER_HEIGHT);
	Vector3 bottomBottomLeftEnd = proposedPosition	+ Vector3(-COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_HALF_PLAYER_HEIGHT);
	Vector3 bottomBottomRightStart = m_position		+ Vector3( COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_HALF_PLAYER_HEIGHT);
	Vector3 bottomBottomRightEnd = proposedPosition	+ Vector3( COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_HALF_PLAYER_HEIGHT);
															   									  
	Vector3 topBottomLeftStart = m_position			+ Vector3(-COLLISION_AVOIDANCE_PLAYER_WIDTH,  COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_HALF_PLAYER_HEIGHT);
	Vector3 topBottomLeftEnd = proposedPosition		+ Vector3(-COLLISION_AVOIDANCE_PLAYER_WIDTH,  COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_HALF_PLAYER_HEIGHT);
	Vector3 topBottomRightStart = m_position		+ Vector3( COLLISION_AVOIDANCE_PLAYER_WIDTH,  COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_HALF_PLAYER_HEIGHT);
	Vector3 topBottomRightEnd = proposedPosition	+ Vector3( COLLISION_AVOIDANCE_PLAYER_WIDTH,  COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_HALF_PLAYER_HEIGHT);
															   									  
	Vector3 bottomMiddleLeftStart = m_position		+ Vector3(-COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_PLAYER_WIDTH, 0.0f);
	Vector3 bottomMiddleLeftEnd = proposedPosition	+ Vector3(-COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_PLAYER_WIDTH, 0.0f);
	Vector3 bottomMiddleRightStart = m_position		+ Vector3( COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_PLAYER_WIDTH, 0.0f);
	Vector3 bottomMiddleRightEnd = proposedPosition	+ Vector3( COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_PLAYER_WIDTH, 0.0f);
															   									  
	Vector3 topMiddleLeftStart = m_position			+ Vector3(-COLLISION_AVOIDANCE_PLAYER_WIDTH,  COLLISION_AVOIDANCE_PLAYER_WIDTH, 0.0f);
	Vector3 topMiddleLeftEnd = proposedPosition		+ Vector3(-COLLISION_AVOIDANCE_PLAYER_WIDTH,  COLLISION_AVOIDANCE_PLAYER_WIDTH, 0.0f);
	Vector3 topMiddleRightStart = m_position		+ Vector3( COLLISION_AVOIDANCE_PLAYER_WIDTH,  COLLISION_AVOIDANCE_PLAYER_WIDTH, 0.0f);
	Vector3 topMiddleRightEnd = proposedPosition	+ Vector3( COLLISION_AVOIDANCE_PLAYER_WIDTH,  COLLISION_AVOIDANCE_PLAYER_WIDTH, 0.0f);
															   									  
	Vector3 bottomTopLeftStart = m_position			+ Vector3(-COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_PLAYER_WIDTH,  COLLISION_AVOIDANCE_HALF_PLAYER_HEIGHT);
	Vector3 bottomTopLeftEnd = proposedPosition		+ Vector3(-COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_PLAYER_WIDTH,  COLLISION_AVOIDANCE_HALF_PLAYER_HEIGHT);
	Vector3 bottomTopRightStart = m_position		+ Vector3( COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_PLAYER_WIDTH,  COLLISION_AVOIDANCE_HALF_PLAYER_HEIGHT);
	Vector3 bottomTopRightEnd = proposedPosition	+ Vector3( COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_PLAYER_WIDTH,  COLLISION_AVOIDANCE_HALF_PLAYER_HEIGHT);
															   									  
	Vector3 topTopLeftStart = m_position			+ Vector3(-COLLISION_AVOIDANCE_PLAYER_WIDTH,  COLLISION_AVOIDANCE_PLAYER_WIDTH,  COLLISION_AVOIDANCE_HALF_PLAYER_HEIGHT);
	Vector3 topTopLeftEnd = proposedPosition		+ Vector3(-COLLISION_AVOIDANCE_PLAYER_WIDTH,  COLLISION_AVOIDANCE_PLAYER_WIDTH,  COLLISION_AVOIDANCE_HALF_PLAYER_HEIGHT);
	Vector3 topTopRightStart = m_position			+ Vector3( COLLISION_AVOIDANCE_PLAYER_WIDTH,  COLLISION_AVOIDANCE_PLAYER_WIDTH,  COLLISION_AVOIDANCE_HALF_PLAYER_HEIGHT);
	Vector3 topTopRightEnd = proposedPosition		+ Vector3( COLLISION_AVOIDANCE_PLAYER_WIDTH,  COLLISION_AVOIDANCE_PLAYER_WIDTH,  COLLISION_AVOIDANCE_HALF_PLAYER_HEIGHT);
	

	RaycastResult3D bottomBottomLeftCast	= m_world->Raycast(bottomBottomLeftStart,	bottomBottomLeftEnd);
	RaycastResult3D bottomBottomRightCast	= m_world->Raycast(bottomBottomRightStart,	bottomBottomRightEnd);
	RaycastResult3D topBottomLeftCast		= m_world->Raycast(topBottomLeftStart,		topBottomLeftEnd);
	RaycastResult3D topBottomRightCast		= m_world->Raycast(topBottomRightStart,		topBottomRightEnd);

	RaycastResult3D bottomMiddleLeftCast	= m_world->Raycast(bottomMiddleLeftStart,	bottomMiddleLeftEnd);
	RaycastResult3D bottomMiddleRightCast	= m_world->Raycast(bottomMiddleRightStart,	bottomMiddleRightEnd);
	RaycastResult3D topMiddleLeftCast		= m_world->Raycast(topMiddleLeftStart,		topMiddleLeftEnd);
	RaycastResult3D topMiddleRightCast		= m_world->Raycast(topMiddleRightStart,		topMiddleRightEnd);

	RaycastResult3D bottomTopLeftCast		= m_world->Raycast(bottomTopLeftStart,		bottomTopLeftEnd);
	RaycastResult3D bottomTopRightCast		= m_world->Raycast(bottomTopRightStart,		bottomTopRightEnd);
	RaycastResult3D topTopLeftCast			= m_world->Raycast(topTopLeftStart,			topTopLeftEnd);
	RaycastResult3D topTopRightCast			= m_world->Raycast(topTopRightStart,		topTopRightEnd);

	result = bottomBottomLeftCast.impactFraction < result.impactFraction ? bottomBottomLeftCast : result;
	result = bottomBottomRightCast.impactFraction < result.impactFraction ? bottomBottomRightCast : result;
	result = topBottomLeftCast.impactFraction < result.impactFraction ? topBottomLeftCast : result;
	result = topBottomRightCast.impactFraction < result.impactFraction ? topBottomRightCast : result;

	result = bottomMiddleLeftCast.impactFraction < result.impactFraction ? bottomMiddleLeftCast : result;
	result = bottomMiddleRightCast.impactFraction < result.impactFraction ? bottomMiddleRightCast : result;
	result = topMiddleLeftCast.impactFraction < result.impactFraction ? topMiddleLeftCast : result;
	result = topMiddleRightCast.impactFraction < result.impactFraction ? topMiddleRightCast : result;

	result = bottomTopLeftCast.impactFraction < result.impactFraction ? bottomTopLeftCast : result;
	result = bottomTopRightCast.impactFraction < result.impactFraction ? bottomTopRightCast : result;
	result = topTopLeftCast.impactFraction < result.impactFraction ? topTopLeftCast : result;
	result = topTopRightCast.impactFraction < result.impactFraction ? topTopRightCast : result;

	return result;
}

void Player::LineOfSightRaycast(float deltaTime)
{
	Vector3 eyeAdjustedPosition = m_position + EYE_OFFSET;
	WorldPosition target = eyeAdjustedPosition + (m_camera.GetForwardXYZ() * 8.0f);
	m_raycastResult = m_world->Raycast(eyeAdjustedPosition, target);
	m_isDigging = false;

	if (m_raycastResult.didImpact)
	{
		if (InputSystem::instance->WasMouseButtonJustPressed(0))
		{
			if (m_physicsMode == PhysicsMode::NOCLIP)
			{
				m_world->DestroyBlock();
				return;
			}
			m_breakingBlockTimer = 0.0f;
			m_blockBeingBroken = m_world->GetBlockInfoFromWorldCoords(m_raycastResult.impactTileCoords);
			m_blockBreakAnimation->SetDuration(m_blockBeingBroken.GetBlock()->GetDefinition()->m_toughness);
			m_blockBreakAnimation->Restart();
			m_blockBreakAnimation->Pause();
		}
		if (InputSystem::instance->IsMouseButtonDown(0))
		{
			m_blockBreakAnimation->Resume();
			Block* targetedBlock = m_world->GetBlockFromWorldCoords(m_raycastResult.impactTileCoords);
			Block* block = m_blockBeingBroken.GetBlock();
			if (!block)
			{
				return;
			}
			if (block != targetedBlock)
			{
				m_breakingBlockTimer = 0.0f;
				m_blockBeingBroken = m_world->GetBlockInfoFromWorldCoords(m_raycastResult.impactTileCoords);
				m_blockBreakAnimation->SetDuration(m_blockBeingBroken.GetBlock()->GetDefinition()->m_toughness);
				m_blockBreakAnimation->Restart();
			}
			Face selectedFace = block->GetFace(m_raycastResult.impactTileCoords, m_raycastResult.impactSurfaceNormal);
			m_isDigging = true;
			m_breakingBlockTimer += deltaTime;
			float toughness = block->GetDefinition()->m_toughness;
			if ((toughness != 0.0f) && (m_breakingBlockTimer / toughness >= 1.0f))
			{
				m_world->DestroyBlock();
				m_breakingBlockTimer = 0.0f;
			}
		}
		else if (InputSystem::instance->WasMouseButtonJustPressed(1))
		{
			m_world->PlaceBlock();
		}
	}

	//Raycast Debug
	if (!InputSystem::instance->IsKeyDown('C'))
	{
		if (m_raycastResult.didImpact)
		{
			m_raycastColor = RGBA::RED;
			m_raycastImpact = m_raycastResult.impactPosition;
			m_target = target;
			m_playerEyes = m_position + EYE_OFFSET;
		}
		else
		{
			m_raycastColor = RGBA::GREEN;
			m_raycastImpact = target;
			m_target = target;
			m_playerEyes = m_position + EYE_OFFSET;
		}
	}
}

void Player::UpdateFromKeyboard(float deltaTime)
{
	SelectBlockFromInventory();

	if (InputSystem::instance->WasKeyJustPressed('L'))
	{
		AddDebugPoint(m_position, RGBA::ORANGE);
	}
	if (InputSystem::instance->WasKeyJustPressed('O'))
	{
		ClearDebugPoints();
	}
	if (InputSystem::instance->WasKeyJustPressed('B'))
	{
		m_drawBoundingBox = !m_drawBoundingBox;
	}
	if (InputSystem::instance->WasKeyJustPressed('V'))
	{
		switch (m_physicsMode)
		{
		case Player::PhysicsMode::NOCLIP:
			m_physicsMode = PhysicsMode::FLYING;
			m_acceleration = Vector3::ZERO;
			m_velocity = Vector3::ZERO;
			break;
		case Player::PhysicsMode::FLYING:
			m_physicsMode = PhysicsMode::NORMAL;
			m_velocity = Vector3::ZERO;
			break;
		case Player::PhysicsMode::NORMAL:
			m_physicsMode = PhysicsMode::NOCLIP;
			m_acceleration = Vector3::ZERO;
			m_velocity = Vector3::ZERO;
			break;
		default:
			break;
		}
	}

	if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::F5))
	{
		switch (m_camera.m_cameraMode)
		{
		case Camera3D::CameraMode::FIRST_PERSON:
			m_camera.m_cameraMode = Camera3D::CameraMode::FROM_BEHIND;
			break;
		case Camera3D::CameraMode::FROM_BEHIND:
			m_camera.m_cameraMode = Camera3D::CameraMode::FIXED_ANGLE_TRACKING;
			break;
		case Camera3D::CameraMode::FIXED_ANGLE_TRACKING:
			m_camera.m_cameraMode = Camera3D::CameraMode::FIRST_PERSON;
			break;
		default:
			break;
		}
	}

	MoveFromKeyboard(deltaTime);

	InputSystem::instance->HideMouseCursor();
	Vector2Int cursorDelta = InputSystem::instance->GetDeltaMouse();

	m_camera.m_position = m_position + EYE_OFFSET;
	m_camera.m_orientation.yawDegreesAboutZ -= ((float)cursorDelta.x * 0.022f);
	float proposedPitch = m_camera.m_orientation.pitchDegreesAboutY + ((float)cursorDelta.y * 0.022f);
	m_camera.m_orientation.pitchDegreesAboutY = MathUtils::Clamp(proposedPitch, -89.9f, 89.9f);
}

void Player::MoveFromKeyboard(float deltaTime)
{
	const float BASE_MOVE_SPEED = 4.5f;
	const float BASE_JUMP_HEIGHT = 5.5f;
	float moveSpeed = 0.0f;
	Vector3 potentialOffset = Vector3::ZERO;

	if (InputSystem::instance->IsKeyDown(InputSystem::ExtraKeys::SHIFT))
	{
		moveSpeed = BASE_MOVE_SPEED * 8.0f;
	}
	else
	{
		moveSpeed = BASE_MOVE_SPEED;
	}
	if (InputSystem::instance->IsKeyDown('W'))
	{
		Vector3 cameraForwardXY = m_camera.GetForwardXY();
		if (m_physicsMode == PhysicsMode::FLYING)
		{
			m_velocity += cameraForwardXY;
			m_velocity = Vector3::GetNormalized(m_velocity) * moveSpeed;
		}
		else
		{
			potentialOffset += cameraForwardXY * (moveSpeed * deltaTime);
		}
	}
	if (InputSystem::instance->IsKeyDown('S'))
	{
		Vector3 cameraForwardXY = m_camera.GetForwardXY();
		if (m_physicsMode == PhysicsMode::FLYING)
		{
			m_velocity -= cameraForwardXY;
			m_velocity = Vector3::GetNormalized(m_velocity) * moveSpeed;
		}
		else
		{
			potentialOffset -= cameraForwardXY * (moveSpeed * deltaTime);
		}
	}
	if (InputSystem::instance->IsKeyDown('D'))
	{
		Vector3 cameraLeftXY = m_camera.GetLeftXY();
		if (m_physicsMode == PhysicsMode::FLYING)
		{
			m_velocity -= cameraLeftXY;
			m_velocity = Vector3::GetNormalized(m_velocity) * moveSpeed;
		}
		else
		{
			potentialOffset -= cameraLeftXY * (moveSpeed * deltaTime);
		}
	}
	if (InputSystem::instance->IsKeyDown('A'))
	{
		Vector3 cameraLeftXY = m_camera.GetLeftXY();
		if (m_physicsMode == PhysicsMode::FLYING)
		{
			m_velocity += cameraLeftXY;
			m_velocity = Vector3::GetNormalized(m_velocity) * moveSpeed;
		}
		else
		{
			potentialOffset += cameraLeftXY * (moveSpeed * deltaTime);
		}
	}
	if (InputSystem::instance->IsKeyDown(' '))
	{
		if (m_physicsMode == PhysicsMode::NORMAL)
		{
			if (IsOnGround())
			{
				m_velocity += Vector3::UNIT_Z * BASE_JUMP_HEIGHT;
			}
		}
		else if (m_physicsMode == PhysicsMode::FLYING)
		{
			m_velocity += Vector3::UNIT_Z;
			m_velocity = Vector3::GetNormalized(m_velocity) * moveSpeed;
		}
		else
		{
			potentialOffset += Vector3(0.f, 0.f, 1.f) * (moveSpeed * deltaTime);
		}
	}
	if (InputSystem::instance->IsKeyDown('Z') && m_physicsMode != PhysicsMode::NORMAL)
	{
		if (m_physicsMode == PhysicsMode::FLYING)
		{
			m_velocity -= Vector3::UNIT_Z;
			m_velocity = Vector3::GetNormalized(m_velocity) * moveSpeed;
		}
		else
		{
			potentialOffset += Vector3(0.f, 0.f, -1.f) * (moveSpeed * deltaTime);
		}
	}

	UpdatePhysics(deltaTime, potentialOffset);
}

void Player::UpdatePhysics(float deltaTime, Vector3 &potentialOffset)
{
	if (m_physicsMode == PhysicsMode::NORMAL)
	{
		m_acceleration = ACCELERATION_DUE_TO_GRAVITY;
	}
	if (m_physicsMode == PhysicsMode::FLYING)
	{
		m_velocity *= 0.90f;
	}
	m_velocity += m_acceleration * deltaTime;
	potentialOffset += m_velocity * deltaTime;

	if (m_physicsMode != PhysicsMode::NOCLIP && potentialOffset != Vector3::ZERO)
	{
		float remainingDistance = 1.0f;
		while (remainingDistance > 0.0f)
		{
			RaycastResult3D shortestRaycast = RaycastForMovementCheck(m_position + potentialOffset);
			if (!shortestRaycast.didImpact)
			{
				m_position += potentialOffset;
				remainingDistance = 0.0f;
			}
			else
			{
				Vector3 validMovement = (potentialOffset * (shortestRaycast.impactFraction - 0.0001f));
				if (validMovement == Vector3::ZERO)
				{
					remainingDistance = 0.0f;
				}
				Vector3 collisionNorm = shortestRaycast.impactSurfaceNormal;
				if (collisionNorm == Vector3::ZERO)
				{
					collisionNorm = Vector3::GetNormalized(potentialOffset);
				}
				m_position += validMovement + (collisionNorm * COLLISION_AVOIDANCE_OFFSET);
				remainingDistance -= shortestRaycast.impactFraction;
				m_velocity = MathUtils::RemoveDirectionalComponent(m_velocity, collisionNorm);
				if (collisionNorm.x != 0.0f)
				{
					potentialOffset.x = 0.0f;
				}
				if (collisionNorm.y != 0.0f)
				{
					potentialOffset.y = 0.0f;
				}
				if (collisionNorm.z != 0.0f)
				{
					potentialOffset.z = 0.0f;
				}
			}
		}
	}
	else
	{
		m_position += potentialOffset;
	}
}

void Player::SelectBlockFromInventory()
{
	const int MAX_SCROLL_AMOUNT = (int)BlockType::NUM_BLOCKS - NUM_INVENTORY_SLOTS;
	int numLinesScrolled = InputSystem::instance->GetScrollAmountThisFrame();
	if (numLinesScrolled != 0)
	{
		m_firstBlockInInventory = (BlockType)MathUtils::Clamp(m_firstBlockInInventory + numLinesScrolled, 1, MAX_SCROLL_AMOUNT);
		AudioSystem::instance->PlaySound(m_uiSelectSound);
	}
	if (InputSystem::instance->WasKeyJustPressed('1'))
	{
		m_highlightedUIBlock = 0;
		AudioSystem::instance->PlaySound(m_uiSelectSound);
	}
	if (InputSystem::instance->WasKeyJustPressed('2'))
	{
		m_highlightedUIBlock = 1;
		AudioSystem::instance->PlaySound(m_uiSelectSound);
	}
	if (InputSystem::instance->WasKeyJustPressed('3'))
	{
		m_highlightedUIBlock = 2;
		AudioSystem::instance->PlaySound(m_uiSelectSound);
	}
	if (InputSystem::instance->WasKeyJustPressed('4'))
	{
		m_highlightedUIBlock = 3;
		AudioSystem::instance->PlaySound(m_uiSelectSound);
	}
	if (InputSystem::instance->WasKeyJustPressed('5'))
	{
		m_highlightedUIBlock = 4;
		AudioSystem::instance->PlaySound(m_uiSelectSound);
	}
	if (InputSystem::instance->WasKeyJustPressed('6'))
	{
		m_highlightedUIBlock = 5;
		AudioSystem::instance->PlaySound(m_uiSelectSound);
	}
	if (InputSystem::instance->WasKeyJustPressed('7'))
	{
		m_highlightedUIBlock = 6;
		AudioSystem::instance->PlaySound(m_uiSelectSound);
	}
	if (InputSystem::instance->WasKeyJustPressed('8'))
	{
		m_highlightedUIBlock = 7;
		AudioSystem::instance->PlaySound(m_uiSelectSound);
	}
	if (InputSystem::instance->WasKeyJustPressed('9'))
	{
		m_highlightedUIBlock = 8;
		AudioSystem::instance->PlaySound(m_uiSelectSound);
	}
	m_heldBlock = (uchar)(m_firstBlockInInventory + m_highlightedUIBlock);
}

bool Player::IsOnGround()
{
	Vector3 bottomBottomLeftStart = m_position + Vector3(-COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_HALF_PLAYER_HEIGHT);
	Vector3 bottomBottomRightStart = m_position + Vector3(COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_HALF_PLAYER_HEIGHT);
	Vector3 topBottomLeftStart = m_position + Vector3(-COLLISION_AVOIDANCE_PLAYER_WIDTH, COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_HALF_PLAYER_HEIGHT);
	Vector3 topBottomRightStart = m_position + Vector3(COLLISION_AVOIDANCE_PLAYER_WIDTH, COLLISION_AVOIDANCE_PLAYER_WIDTH, -COLLISION_AVOIDANCE_HALF_PLAYER_HEIGHT);

	RaycastResult3D bottomBottomLeftCast = m_world->Raycast(bottomBottomLeftStart, bottomBottomLeftStart - Vector3::UNIT_Z);
	RaycastResult3D bottomBottomRightCast = m_world->Raycast(bottomBottomRightStart, bottomBottomRightStart - Vector3::UNIT_Z);
	RaycastResult3D topBottomLeftCast = m_world->Raycast(topBottomLeftStart, topBottomLeftStart - Vector3::UNIT_Z);
	RaycastResult3D topBottomRightCast = m_world->Raycast(topBottomRightStart, topBottomRightStart - Vector3::UNIT_Z);

	bool isAboveGroundBlock = (bottomBottomLeftCast.impactSurfaceNormal == Vector3Int::UNIT_Z) || (bottomBottomRightCast.impactSurfaceNormal == Vector3Int::UNIT_Z)
		|| (topBottomLeftCast.impactSurfaceNormal == Vector3Int::UNIT_Z) || (topBottomRightCast.impactSurfaceNormal == Vector3Int::UNIT_Z);

	return isAboveGroundBlock && ((bottomBottomLeftCast.impactFraction <= COLLISION_AVOIDANCE_OFFSET) || (bottomBottomRightCast.impactFraction <= COLLISION_AVOIDANCE_OFFSET)
		|| (topBottomLeftCast.impactFraction <= COLLISION_AVOIDANCE_OFFSET) || (topBottomRightCast.impactFraction <= COLLISION_AVOIDANCE_OFFSET));
}
