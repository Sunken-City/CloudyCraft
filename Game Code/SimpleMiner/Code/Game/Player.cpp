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

//-----------------------------------------------------------------------------------
Player::Player(World* world, PhysicsMode physicsMode /*= PhysicsMode::NOCLIP*/)
: m_camera(Camera3D(this))
, m_world(world)
, m_physicsMode(physicsMode)
, m_position(8.f, 8.f, 100.f)
, m_velocity(0.0f, 0.0f, 0.0f)
, m_acceleration(0.0f, 0.0f, 0.0f)
, m_boundingBox(Vector3(-HALF_PLAYER_WIDTH, -HALF_PLAYER_WIDTH, -HALF_PLAYER_HEIGHT), Vector3(HALF_PLAYER_WIDTH, HALF_PLAYER_WIDTH, HALF_PLAYER_HEIGHT))
, m_heldBlock(BlockType::SAND)
, m_texture(Texture::CreateOrGetTexture("Data/Images/Pico.png"))
, m_breakingBlockTimer(0.0f)
, m_blockBreakAnimation(new SpriteAnim(*BlockDefinition::m_blockSheet, 1.0f, AnimMode::ONE_SHOT, 0xE0, 0xE9))
, m_isDigging(false)
, m_uiSelectSound(AudioSystem::instance->CreateOrGetSound("Data/SFX/select.wav"))
, m_highlightedUIBlock(4) //5th slot, the middle entry
, m_firstBlockInInventory(BlockType::STONE)
{
}

//-----------------------------------------------------------------------------------
Player::~Player()
{
    delete m_blockBreakAnimation;
}

//-----------------------------------------------------------------------------------
void Player::Update(float deltaTime)
{
    UpdateFromKeyboard(deltaTime);
    m_blockBreakAnimation->Update(deltaTime);
    LineOfSightRaycast(deltaTime);

}

//-----------------------------------------------------------------------------------
void Player::Render() const
{
    if (g_renderDebug)
    {
        RenderPlayerBoundingBox();
        //Raycast Debug
        Renderer::instance->DrawLine(m_playerEyes, m_target, m_raycastColor, 5.0f);
        Renderer::instance->EnableDepthTest(false);
        Renderer::instance->DrawLine(m_playerEyes, m_target, m_raycastColor, 1.0f);
    }

    //Highlight face when impacted.
    if (m_raycastResult.didImpact)
    {
        RenderFaceHighlightOnRaycastBlock();
        if (m_isDigging)
        {
            RenderDiggingAnimation();
        }
    }
    if (m_camera.m_cameraMode != Camera3D::CameraMode::FIRST_PERSON)
    {
        Render2DPlayerTexture();
    }
}

//-----------------------------------------------------------------------------------
void Player::RenderDiggingAnimation() const
{
    Renderer::instance->EnableDepthTest(true);
    AABB2 texCoords = m_blockBreakAnimation->GetCurrentTexCoords();

    //replace this with a difference in my position and the block's to determine which faces are visible.
    Face west = Block::GetFace(m_raycastResult.impactTileCoords, Vector3Int::UNIT_X);
    Face east = Block::GetFace(m_raycastResult.impactTileCoords, Vector3Int::UNIT_X * -1);
    Face north = Block::GetFace(m_raycastResult.impactTileCoords, Vector3Int::UNIT_Y);
    Face south = Block::GetFace(m_raycastResult.impactTileCoords, Vector3Int::UNIT_Y * -1);
    Face top = Block::GetFace(m_raycastResult.impactTileCoords, Vector3Int::UNIT_Z);
    Face bottom = Block::GetFace(m_raycastResult.impactTileCoords, Vector3Int::UNIT_Z * -1);

    Renderer::instance->DrawTexturedFace(west, texCoords.mins, texCoords.maxs, m_blockBreakAnimation->GetTexture(), RGBA(1.0f, 1.0f, 1.0f, 0.2f));
    Renderer::instance->DrawTexturedFace(east, texCoords.mins, texCoords.maxs, m_blockBreakAnimation->GetTexture(), RGBA(1.0f, 1.0f, 1.0f, 0.2f));
    Renderer::instance->DrawTexturedFace(north, texCoords.mins, texCoords.maxs, m_blockBreakAnimation->GetTexture(), RGBA(1.0f, 1.0f, 1.0f, 0.2f));
    Renderer::instance->DrawTexturedFace(south, texCoords.mins, texCoords.maxs, m_blockBreakAnimation->GetTexture(), RGBA(1.0f, 1.0f, 1.0f, 0.2f));
    Renderer::instance->DrawTexturedFace(top, texCoords.mins, texCoords.maxs, m_blockBreakAnimation->GetTexture(), RGBA(1.0f, 1.0f, 1.0f, 0.2f));
    Renderer::instance->DrawTexturedFace(bottom, texCoords.mins, texCoords.maxs, m_blockBreakAnimation->GetTexture(), RGBA(1.0f, 1.0f, 1.0f, 0.2f));
}

//-----------------------------------------------------------------------------------
void Player::RenderFaceHighlightOnRaycastBlock() const
{
    Vector3 vertOffset = m_camera.GetForwardXYZ() * -0.01f;
    Renderer::instance->SetLineWidth(2.0f);
    Renderer::instance->EnableDepthTest(true);
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
    Renderer::instance->DrawVertexArray(vertexes.data(), vertexes.size(), Renderer::DrawMode::LINE_LOOP);
}

//-----------------------------------------------------------------------------------
void Player::Render2DPlayerTexture() const
{
    //Render Player
    Vector3 bottomLeft = m_position - (Vector3::UNIT_X * HALF_PLAYER_WIDTH * 1.5f) - (Vector3::UNIT_Z * HALF_PLAYER_HEIGHT);
    Vector3 bottomRight = m_position + (Vector3::UNIT_X * HALF_PLAYER_WIDTH * 1.5f) - (Vector3::UNIT_Z * HALF_PLAYER_HEIGHT);
    Vector3 topLeft = m_position - (Vector3::UNIT_X * HALF_PLAYER_WIDTH * 1.5f) + (Vector3::UNIT_Z * HALF_PLAYER_HEIGHT);
    Vector3 topRight = m_position + (Vector3::UNIT_X * HALF_PLAYER_WIDTH * 1.5f) + (Vector3::UNIT_Z * HALF_PLAYER_HEIGHT);
    Face playerQuad = Face(topRight, topLeft, bottomLeft, bottomRight);
    Renderer::instance->EnableDepthTest(true);
    Renderer::instance->DrawTexturedFace(playerQuad, Vector2::ZERO, Vector2::ONE, m_texture, RGBA::WHITE);
}

//-----------------------------------------------------------------------------------
void Player::RenderPlayerBoundingBox() const
{
    Renderer::instance->EnableDepthTest(true);
    Renderer::instance->SetLineWidth(4.0f);
    Renderer::instance->DrawAABBBoundingBox(m_boundingBox + m_position, RGBA::MAGENTA);
    Renderer::instance->EnableDepthTest(false);
    Renderer::instance->SetLineWidth(1.0f);
    Renderer::instance->DrawAABBBoundingBox(m_boundingBox + m_position, RGBA::MAGENTA);
}

//-----------------------------------------------------------------------------------
RaycastResult3D Player::RaycastForMovementCheck(WorldPosition proposedPosition)
{
    RaycastResult3D result;

    //I am raycasting outward from 12 different points, 4 top and 4 bottom, as well as 4 middle.
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

    m_summedCollision = Vector3Int::ZERO;
    m_summedCollision += bottomBottomLeftCast.impactSurfaceNormal;
    m_summedCollision += bottomBottomRightCast.impactSurfaceNormal;
    m_summedCollision += topBottomLeftCast.impactSurfaceNormal;
    m_summedCollision += topBottomRightCast.impactSurfaceNormal;
    m_summedCollision += bottomMiddleLeftCast.impactSurfaceNormal;
    m_summedCollision += bottomMiddleRightCast.impactSurfaceNormal;
    m_summedCollision += topMiddleLeftCast.impactSurfaceNormal;
    m_summedCollision += topMiddleRightCast.impactSurfaceNormal;
    m_summedCollision += bottomTopLeftCast.impactSurfaceNormal;
    m_summedCollision += bottomTopRightCast.impactSurfaceNormal;
    m_summedCollision += topTopLeftCast.impactSurfaceNormal;
    m_summedCollision += topTopRightCast.impactSurfaceNormal;

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

    m_summedCollision.x /= (m_summedCollision.x == 0) ? 1 : m_summedCollision.x;
    m_summedCollision.y /= (m_summedCollision.y == 0) ? 1 : m_summedCollision.y;
    m_summedCollision.z /= (m_summedCollision.z == 0) ? 1 : m_summedCollision.z;
    return result;
}

//-----------------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------------
void Player::UpdateFromKeyboard(float deltaTime)
{
    SelectBlockFromInventory();

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
            m_physicsMode = PhysicsMode::WALKING;
            m_velocity = Vector3::ZERO;
            break;
        case Player::PhysicsMode::WALKING:
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
}

//-----------------------------------------------------------------------------------
void Player::MoveFromKeyboard(float deltaTime)
{
    const float BASE_MOVE_SPEED = 6.0f;
    const float BASE_JUMP_HEIGHT = 5.5f;
    const float MAX_HORIZONTAL_VELOCITY = 10.0f;
    const float SHIFT_SPEED_MULTIPLIER = 4.0f;
    float moveSpeed = 0.0f;
    moveSpeed = (InputSystem::instance->IsKeyDown(InputSystem::ExtraKeys::SHIFT)) ? BASE_MOVE_SPEED * SHIFT_SPEED_MULTIPLIER : moveSpeed = BASE_MOVE_SPEED;

    switch (m_physicsMode)
    {
    case PhysicsMode::NOCLIP:
    {
        if (InputSystem::instance->IsKeyDown('W')) { m_position += m_camera.GetForwardXY() * (moveSpeed * deltaTime); }
        if (InputSystem::instance->IsKeyDown('S')) { m_position -= m_camera.GetForwardXY() * (moveSpeed * deltaTime); }
        if (InputSystem::instance->IsKeyDown('A')) { m_position += m_camera.GetLeftXY() * (moveSpeed * deltaTime); }
        if (InputSystem::instance->IsKeyDown('D')) { m_position -= m_camera.GetLeftXY() * (moveSpeed * deltaTime); }
        if (InputSystem::instance->IsKeyDown(' ')) { m_position += Vector3::UNIT_Z * (moveSpeed * deltaTime); }
        if (InputSystem::instance->IsKeyDown('Z')) { m_position -= Vector3::UNIT_Z * (moveSpeed * deltaTime); }
        break;
    }
    case PhysicsMode::FLYING:
    {
        if (InputSystem::instance->IsKeyDown('W')) { m_velocity += Vector3::GetNormalized(m_camera.GetForwardXY()) * (moveSpeed / 8.0f); }
        if (InputSystem::instance->IsKeyDown('S')) { m_velocity -= Vector3::GetNormalized(m_camera.GetForwardXY()) * (moveSpeed / 8.0f); }
        if (InputSystem::instance->IsKeyDown('A')) { m_velocity += Vector3::GetNormalized(m_camera.GetLeftXY()) * (moveSpeed / 8.0f); }
        if (InputSystem::instance->IsKeyDown('D')) { m_velocity -= Vector3::GetNormalized(m_camera.GetLeftXY()) * (moveSpeed / 8.0f); }
        if (InputSystem::instance->IsKeyDown(' ')) { m_velocity += Vector3::UNIT_Z * (moveSpeed / 8.0f); }
        if (InputSystem::instance->IsKeyDown('Z')) { m_velocity -= Vector3::UNIT_Z * (moveSpeed / 8.0f); }
        break;
    }
    case PhysicsMode::WALKING:
    {
        Vector3 inputVector = Vector3::ZERO;
        if (InputSystem::instance->IsKeyDown('W')) { inputVector += Vector3::GetNormalized(m_camera.GetForwardXY()); }
        if (InputSystem::instance->IsKeyDown('S')) { inputVector -= Vector3::GetNormalized(m_camera.GetForwardXY()); }
        if (InputSystem::instance->IsKeyDown('A')) { inputVector += Vector3::GetNormalized(m_camera.GetLeftXY()); }
        if (InputSystem::instance->IsKeyDown('D')) { inputVector -= Vector3::GetNormalized(m_camera.GetLeftXY()); }
        if (InputSystem::instance->IsKeyDown(' ') && IsOnGround()) { m_velocity += Vector3::UNIT_Z * BASE_JUMP_HEIGHT; }
        Vector3 velocity = Vector3::GetNormalized(inputVector) * moveSpeed;
        m_velocity.x = velocity.x;
        m_velocity.y = velocity.y;
        break;
    }
    default:
        break;
    }
    UpdatePhysics(deltaTime);
}

//-----------------------------------------------------------------------------------
void Player::UpdatePhysics(float deltaTime)
{
    if (m_physicsMode == PhysicsMode::WALKING)
    {
        m_acceleration = ACCELERATION_DUE_TO_GRAVITY;
        //Have our speed fall off whenever we move.
        m_velocity.x *= 0.90f; 
        m_velocity.y *= 0.90f; 
    }
    if (m_physicsMode == PhysicsMode::FLYING)
    {
        //Have our speed fall off whenever we move.
        m_velocity *= 0.90f; 
    }
    m_velocity += m_acceleration * deltaTime;
    if (m_physicsMode != PhysicsMode::NOCLIP && m_velocity != Vector3::ZERO)
    {
        Vector3Int TheRealSummedCollision = Vector3Int::ZERO;
        float secondsLeft = deltaTime;
        while (secondsLeft > 0.0f)
        {
            Vector3 deltaMove = m_velocity * secondsLeft;
            RaycastResult3D shortestRaycast = RaycastForMovementCheck(m_position + deltaMove);
            if (!shortestRaycast.didImpact)
            {
                m_position += deltaMove;
                secondsLeft = 0.0f;
            }
            else if (shortestRaycast.wasInsideBlockAlready == true)
            {
                m_position += ((Vector3)(shortestRaycast.impactSurfaceNormal) * COLLISION_AVOIDANCE_OFFSET * 0.5f);
                secondsLeft = 0.0f;
            }
            else
            {
                Vector3 validMovement = (deltaMove * (shortestRaycast.impactFraction));
                if (validMovement == Vector3::ZERO)
                {
                    secondsLeft = 0.0f;
                }
                Vector3 collisionNorm = shortestRaycast.impactSurfaceNormal;
                if (collisionNorm == Vector3::ZERO)
                {
                    collisionNorm = Vector3::GetNormalized(deltaMove);
                }
                m_position += validMovement + (collisionNorm * COLLISION_AVOIDANCE_OFFSET);
                secondsLeft -= shortestRaycast.impactFraction * secondsLeft;
                if (shortestRaycast.impactSurfaceNormal.x != 0.0f)
                {
                    m_velocity.x = 0.0f;
                }
                if (shortestRaycast.impactSurfaceNormal.y != 0.0f)
                {
                    m_velocity.y = 0.0f;
                }
                if (shortestRaycast.impactSurfaceNormal.z != 0.0f)
                {
                    m_velocity.z = 0.0f;
                }
                if (m_velocity == Vector3::ZERO)
                {
                    secondsLeft = 0.0f;
                }
                TheRealSummedCollision = m_summedCollision;
            }
            RaycastResult3D playerPositionResult = m_world->Raycast(m_position, m_position + deltaMove);
            if (playerPositionResult.impactedBlockInfo.IsValid() && playerPositionResult.impactedBlockInfo.m_chunk->m_world != m_world)
            {
                TheGame::instance->SwapWorlds();
            }
        }
    }
}

//-----------------------------------------------------------------------------------
void Player::SelectBlockFromInventory()
{
    const int MAX_SCROLL_AMOUNT = (int)BlockType::NUM_BLOCKS - NUM_INVENTORY_SLOTS;
    int numLinesScrolled = InputSystem::instance->GetScrollAmountThisFrame();
    if (numLinesScrolled != 0)
    {
        int newFirstIndex = (m_firstBlockInInventory + numLinesScrolled);
        //Wrap around positive and negative directions
        newFirstIndex           = newFirstIndex >= (int)BlockType::NUM_BLOCKS ? (m_firstBlockInInventory + numLinesScrolled) % (int)BlockType::NUM_BLOCKS : newFirstIndex;
        m_firstBlockInInventory = (BlockType)(newFirstIndex < 0 ? ((int)BlockType::NUM_BLOCKS) + numLinesScrolled : newFirstIndex);

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
    m_heldBlock = (uchar)((m_firstBlockInInventory + m_highlightedUIBlock) % (int)BlockType::NUM_BLOCKS);
}

//-----------------------------------------------------------------------------------
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
