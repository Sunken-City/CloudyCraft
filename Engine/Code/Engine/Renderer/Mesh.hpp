#pragma once
#include <vector>
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/Renderer.hpp"

class Vector3Int;
class Vector3;
class ShaderProgram;
class Material;
struct Vertex_PCUTB;


class Mesh
{
	typedef unsigned int GLuint;
public:
	typedef void (BindMeshToVAOForVertex)(GLuint vao, GLuint vbo, GLuint ibo, ShaderProgram* program);
	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
	Mesh();
	~Mesh();

	//FUNCTIONS//////////////////////////////////////////////////////////////////////////
	void RenderFromIBO(GLuint vaoID, Material* material) const;

	//HELPER FUNCTIONS//////////////////////////////////////////////////////////////////////////
	void Init(void* vertexData, unsigned int numVertices, unsigned int sizeofVertex, void* indexData, unsigned int numIndices, BindMeshToVAOForVertex* BindMeshFunction);
	void BindToVAO(GLuint m_vaoID, ShaderProgram* m_shaderProgram);

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	GLuint m_vbo;
	GLuint m_ibo;
	unsigned int m_numVerts;
	unsigned int m_numIndices;
	BindMeshToVAOForVertex* m_vertexBindFunctionPointer;
	Renderer::DrawMode m_drawMode;

private:
	Mesh(const Mesh&);
};