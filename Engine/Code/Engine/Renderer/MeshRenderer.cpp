#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Vertex.hpp"

MeshRenderer::MeshRenderer()
{

}

MeshRenderer::MeshRenderer(const Mesh& mesh, Material* material)
	: m_mesh(mesh)
	, m_material(material)
	, m_model(Matrix4x4::IDENTITY)
{
	m_vaoID = Renderer::instance->GenerateVertexArraysHandle();
	Renderer::instance->BindMeshToVAOVertexPCUTB(m_vaoID, m_mesh.m_vbo, m_mesh.m_ibo, m_material->m_shaderProgram);
	GL_CHECK_ERROR();
}

MeshRenderer::~MeshRenderer()
{

}

void MeshRenderer::Render() const
{
	m_material->BindAvailableTextures();
	m_mesh.RenderFromIBO(m_vaoID, *m_material);
	GL_CHECK_ERROR();
}

void MeshRenderer::Render(const Matrix4x4& view, const Matrix4x4& projection) const
{
	m_material->SetMatrices(m_model, view, projection);
	Render();
}

void MeshRenderer::SetPosition(const Vector3& worldPosition)
{
	m_model.SetTranslation(worldPosition);
}

void MeshRenderer::SetVec3Uniform(const char* uniformName, const Vector3& value)
{
	m_material->SetVec3Uniform(uniformName, value);
}
