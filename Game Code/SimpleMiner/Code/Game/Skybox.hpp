#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Renderer/RGBA.hpp"

class Texture;
class MeshRenderer;
struct RenderState;

class Skybox
{
public:
    Skybox() {};
    Skybox(Texture* topTexture, Texture* bottomTexture, Texture* sideTexture, const RGBA& color = RGBA::WHITE);
    ~Skybox();
    
    void RenderAt(const Vector3& position) const;
    void SetRenderState(const RenderState& state) const;

private:
    static const int NUM_FACES = 6;
    static const float SIDE_LENGTH;
    MeshRenderer* topFace;
    MeshRenderer* bottomFace;
    MeshRenderer* rightFace;
    MeshRenderer* forwardFace;
    MeshRenderer* leftFace;
    MeshRenderer* backwardFace;
    MeshRenderer* faces[NUM_FACES];
};