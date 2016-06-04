#include "Game/Skybox.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Math/Vector4.hpp"

const float Skybox::SIDE_LENGTH = 100.0f;

//-----------------------------------------------------------------------------------
Skybox::Skybox(Texture* topTexture, Texture* bottomTexture, Texture* sideTexture, const RGBA& color /*= RGBA::WHITE*/)
{
    RenderState faceRenderState = RenderState(RenderState::DepthTestingMode::OFF, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND);
    const float ADJUSTED_SIDE_LENGTH = SIDE_LENGTH + 1.0f; //Fudged to prevent little edges around the box from showing.

    topFace = new MeshRenderer(new Mesh(), new Material(new ShaderProgram("Data/Shaders/fvfPortal.vert", "Data/Shaders/fvfPortal.frag"), faceRenderState));
    bottomFace = new MeshRenderer(new Mesh(), new Material(new ShaderProgram("Data/Shaders/fvfPortal.vert", "Data/Shaders/fvfPortal.frag"), faceRenderState));
    forwardFace = new MeshRenderer(new Mesh(), new Material(new ShaderProgram("Data/Shaders/fvfPortal.vert", "Data/Shaders/fvfPortal.frag"), faceRenderState));
    backwardFace = new MeshRenderer(new Mesh(), new Material(new ShaderProgram("Data/Shaders/fvfPortal.vert", "Data/Shaders/fvfPortal.frag"), faceRenderState));
    rightFace = new MeshRenderer(new Mesh(), new Material(new ShaderProgram("Data/Shaders/fvfPortal.vert", "Data/Shaders/fvfPortal.frag"), faceRenderState));
    leftFace = new MeshRenderer(new Mesh(), new Material(new ShaderProgram("Data/Shaders/fvfPortal.vert", "Data/Shaders/fvfPortal.frag"), faceRenderState));

    MeshBuilder topFaceBuilder;
    topFaceBuilder.Begin();
    topFaceBuilder.SetFloatData0(Vector4::ZERO);
    topFaceBuilder.AddQuad(Vector3::FORWARD * SIDE_LENGTH + Vector3::UP * SIDE_LENGTH, -Vector3::FORWARD, ADJUSTED_SIDE_LENGTH, Vector3::RIGHT, ADJUSTED_SIDE_LENGTH, color, Vector2::ZERO, 1.0f); //TOP
    topFaceBuilder.End();
    topFaceBuilder.CopyToMesh(topFace->m_mesh, &Vertex_PCTD::Copy, sizeof(Vertex_PCTD), &Vertex_PCTD::BindMeshToVAO);
    topFace->m_material->SetDiffuseTexture(topTexture);

    MeshBuilder bottomFaceBuilder;
    bottomFaceBuilder.Begin();
    bottomFaceBuilder.SetFloatData0(Vector4::ZERO);
    bottomFaceBuilder.AddQuad(Vector3::ZERO, Vector3::FORWARD, ADJUSTED_SIDE_LENGTH, Vector3::RIGHT, ADJUSTED_SIDE_LENGTH, color, Vector2::ZERO, 1.0f); //BOTTOM
    bottomFaceBuilder.End();
    bottomFaceBuilder.CopyToMesh(bottomFace->m_mesh, &Vertex_PCTD::Copy, sizeof(Vertex_PCTD), &Vertex_PCTD::BindMeshToVAO);
    bottomFace->m_material->SetDiffuseTexture(bottomTexture);

    MeshBuilder forwardFaceBuilder;
    forwardFaceBuilder.Begin();
    forwardFaceBuilder.SetFloatData0(Vector4::ZERO);
    forwardFaceBuilder.AddQuad(Vector3::FORWARD * SIDE_LENGTH, Vector3::UP, ADJUSTED_SIDE_LENGTH, Vector3::RIGHT, ADJUSTED_SIDE_LENGTH, color, Vector2::ZERO, 1.0f); //SOUTH
    forwardFaceBuilder.End();
    forwardFaceBuilder.CopyToMesh(forwardFace->m_mesh, &Vertex_PCTD::Copy, sizeof(Vertex_PCTD), &Vertex_PCTD::BindMeshToVAO);
    forwardFace->m_material->SetDiffuseTexture(sideTexture);

    MeshBuilder backwardFaceBuilder;
    backwardFaceBuilder.Begin();
    backwardFaceBuilder.SetFloatData0(Vector4::ZERO);
    backwardFaceBuilder.AddQuad(Vector3::RIGHT * SIDE_LENGTH, Vector3::UP, ADJUSTED_SIDE_LENGTH, -Vector3::RIGHT, ADJUSTED_SIDE_LENGTH, color, Vector2::ZERO, 1.0f); //NORTH
    backwardFaceBuilder.End();
    backwardFaceBuilder.CopyToMesh(backwardFace->m_mesh, &Vertex_PCTD::Copy, sizeof(Vertex_PCTD), &Vertex_PCTD::BindMeshToVAO);
    backwardFace->m_material->SetDiffuseTexture(sideTexture);

    MeshBuilder rightFaceBuilder;
    rightFaceBuilder.Begin();
    rightFaceBuilder.SetFloatData0(Vector4::ZERO);
    rightFaceBuilder.AddQuad(Vector3::FORWARD * SIDE_LENGTH + Vector3::RIGHT * SIDE_LENGTH, Vector3::UP, ADJUSTED_SIDE_LENGTH, -Vector3::FORWARD, ADJUSTED_SIDE_LENGTH, color, Vector2::ZERO, 1.0f); //EAST
    rightFaceBuilder.End();
    rightFaceBuilder.CopyToMesh(rightFace->m_mesh, &Vertex_PCTD::Copy, sizeof(Vertex_PCTD), &Vertex_PCTD::BindMeshToVAO);
    rightFace->m_material->SetDiffuseTexture(sideTexture);

    MeshBuilder leftFaceBuilder;
    leftFaceBuilder.Begin();
    leftFaceBuilder.SetFloatData0(Vector4::ZERO);
    leftFaceBuilder.AddQuad(Vector3::ZERO, Vector3::UP, ADJUSTED_SIDE_LENGTH, Vector3::FORWARD, ADJUSTED_SIDE_LENGTH, color, Vector2::ZERO, 1.0f); //WEST
    leftFaceBuilder.End();
    leftFaceBuilder.CopyToMesh(leftFace->m_mesh, &Vertex_PCTD::Copy, sizeof(Vertex_PCTD), &Vertex_PCTD::BindMeshToVAO);
    leftFace->m_material->SetDiffuseTexture(sideTexture);

    faces[0] = topFace;
    faces[1] = bottomFace;
    faces[2] = forwardFace;
    faces[3] = backwardFace;
    faces[4] = rightFace;
    faces[5] = leftFace;

    for (int i = 0; i < NUM_FACES; ++i)
    {
        //Put Z up.
        faces[i]->RotateAround(90.0f, Vector3::UNIT_X);
        //Put X forward.
        faces[i]->RotateAround(90.0f, Vector3::UNIT_Z);
    }
}

//-----------------------------------------------------------------------------------
Skybox::~Skybox()
{
    delete topFace->m_material->m_shaderProgram;
    delete topFace->m_material;
    delete topFace->m_mesh;
    delete topFace;
    delete bottomFace->m_material->m_shaderProgram;
    delete bottomFace->m_material;
    delete bottomFace->m_mesh;
    delete bottomFace;
    delete forwardFace->m_material->m_shaderProgram;
    delete forwardFace->m_material;
    delete forwardFace->m_mesh;
    delete forwardFace;
    delete backwardFace->m_material->m_shaderProgram;
    delete backwardFace->m_material;
    delete backwardFace->m_mesh;
    delete backwardFace;
    delete rightFace->m_material->m_shaderProgram;
    delete rightFace->m_material;
    delete rightFace->m_mesh;
    delete rightFace;
    delete leftFace->m_material->m_shaderProgram;
    delete leftFace->m_material;
    delete leftFace->m_mesh;
    delete leftFace;
}

//-----------------------------------------------------------------------------------
void Skybox::RenderAt(const Vector3& position) const
{
    const float HALF_SIDE_LENGTH = (SIDE_LENGTH / 2.0f);
    const Vector3 BOX_OFFSET = -(Vector3::ONE * HALF_SIDE_LENGTH);
    for (int i = 0; i < NUM_FACES; ++i)
    {
        faces[i]->SetPosition(position + BOX_OFFSET);
        faces[i]->Render();
    }
}

//-----------------------------------------------------------------------------------
void Skybox::SetRenderState(const RenderState& state) const
{
    faces[0]->m_material->m_renderState = state;
    faces[1]->m_material->m_renderState = state;
    faces[2]->m_material->m_renderState = state;
    faces[3]->m_material->m_renderState = state;
    faces[4]->m_material->m_renderState = state;
    faces[5]->m_material->m_renderState = state;
}
