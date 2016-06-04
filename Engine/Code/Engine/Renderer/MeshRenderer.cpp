#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Vertex.hpp"

//-----------------------------------------------------------------------------------
MeshRenderer::MeshRenderer()
    : m_mesh(nullptr)
    , m_material(nullptr)
    , m_model(Matrix4x4::IDENTITY)
    , m_vaoID(0)
{

}

//-----------------------------------------------------------------------------------
MeshRenderer::MeshRenderer(Mesh* mesh, Material* material)
    : m_mesh(mesh)
    , m_material(material)
    , m_model(Matrix4x4::IDENTITY)
    , m_vaoID(0)
{
    m_vaoID = Renderer::instance->GenerateVAOHandle();
    GL_CHECK_ERROR();
}

//-----------------------------------------------------------------------------------
MeshRenderer::~MeshRenderer()
{
    if (m_vaoID != 0)
    {
        Renderer::instance->DeleteVAOHandle(m_vaoID);
    }
}

//-----------------------------------------------------------------------------------
void MeshRenderer::Render() const
{
    m_material->SetMatrices(m_model, Renderer::instance->m_viewStack.GetTop(), Renderer::instance->m_projStack.GetTop());
    m_material->BindAvailableTextures();
    m_mesh->BindToVAO(m_vaoID, m_material->m_shaderProgram);
    GL_CHECK_ERROR();
    m_mesh->RenderFromIBO(m_vaoID, m_material);
    GL_CHECK_ERROR();
    Renderer::instance->UnbindIbo();
    m_material->UnbindAvailableTextures();
}

//-----------------------------------------------------------------------------------
void MeshRenderer::RotateAround(float degrees, const Vector3& axis)
{
    m_model.Rotate(degrees, axis);
}


//-----------------------------------------------------------------------------------
void MeshRenderer::SetPosition(const Vector3& worldPosition)
{
    m_model.SetTranslation(worldPosition);
}

//-----------------------------------------------------------------------------------
void MeshRenderer::SetVec3Uniform(const char* uniformName, const Vector3& value)
{
    m_material->SetVec3Uniform(uniformName, value);
}
