#pragma once
#include <vector>
#include "Engine/Renderer/RGBA.hpp"

class Vector3Int;
class Vector3;
class ShaderProgram;
class Material;
struct Vertex_PCUTB;

class Mesh
{
	typedef unsigned int GLuint;
public:
	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
	Mesh();
	~Mesh();

	//FUNCTIONS//////////////////////////////////////////////////////////////////////////
	void RenderFromIBO(GLuint vaoID, const Material& program) const;
	static Mesh CreateCube(float sideLength, const RGBA& color = RGBA::WHITE);
	static Mesh CreateUVSphere(float radius, int numSegments, const RGBA& color = RGBA::WHITE);
	static Mesh CreateIcoSphere(float radius, const RGBA& color = RGBA::WHITE, int numPasses = 3);
	static Mesh CreateQuad(const Vector3& bottomLeft, const Vector3& up, float upLength, const Vector3& right, float rightLength, const RGBA& color = RGBA::WHITE);

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	GLuint m_vbo;
	GLuint m_ibo;
private:
	//HELPER FUNCTIONS//////////////////////////////////////////////////////////////////////////
	void Init();
	static void AddQuadToMesh(Mesh& meshBeingConstructed, const Vector3& bottomLeft, const Vector3& up, float upLength, const Vector3& right, float rightLength, const RGBA& color /*= RGBA::WHITE*/);

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	std::vector<Vertex_PCUTB> m_verts;
	std::vector<unsigned int> m_indices;
};