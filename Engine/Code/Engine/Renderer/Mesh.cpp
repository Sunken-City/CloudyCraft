#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Vertex.hpp"
#include "Engine/Math/Vector3Int.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Renderer.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include "Engine/Renderer/OpenGLExtensions.hpp"

//-----------------------------------------------------------------------------------
Mesh::Mesh()
    : m_drawMode(Renderer::DrawMode::TRIANGLES)
	, m_vbo(0)
	, m_ibo(0)
{

}

//-----------------------------------------------------------------------------------
Mesh::~Mesh()
{
	if (m_vbo != 0)
	{
		Renderer::instance->DeleteBuffers(m_vbo);
	}
	if (m_ibo != 0)
	{
		Renderer::instance->RenderBufferDestroy(m_ibo);
	}
}

//-----------------------------------------------------------------------------------
void Mesh::RenderFromIBO(GLuint vaoID, Material* material) const
{
    glBindVertexArray(vaoID);
    material->SetUpRenderState();
    //Draw with IBO
    glDrawElements(Renderer::instance->GetDrawMode(m_drawMode), m_numIndices, GL_UNSIGNED_INT, (GLvoid*)0);
    material->CleanUpRenderState();
    glUseProgram(NULL);
    glBindVertexArray(NULL);
}

//Pushes data over to the GPU and creates the buffers. The mesh doesn't store any of the vertexes or indexes, just the buffer locations.
//-----------------------------------------------------------------------------------
void Mesh::Init(void* vertexData, unsigned int numVertices, unsigned int sizeofVertex, void* indexData, unsigned int numIndices, BindMeshToVAOForVertex* BindMeshFunction)
{
	m_numVerts = numVertices;
	m_numIndices = numIndices;
	m_vertexBindFunctionPointer = BindMeshFunction;
	m_vbo = Renderer::instance->GenerateBufferID();
	GL_CHECK_ERROR();
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeofVertex * numVertices, vertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
	GL_CHECK_ERROR();
	m_ibo = Renderer::instance->RenderBufferCreate(indexData, numIndices, sizeof(unsigned int), GL_STATIC_DRAW);
	GL_CHECK_ERROR();
}

//-----------------------------------------------------------------------------------
void Mesh::BindToVAO(GLuint vaoID, ShaderProgram* shaderProgram)
{
	m_vertexBindFunctionPointer(vaoID, m_vbo, m_ibo, shaderProgram);
}


