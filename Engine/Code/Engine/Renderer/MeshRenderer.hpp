#pragma once
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Math/Matrix4x4.hpp"
#include "Engine/Renderer/Vertex.hpp"

class Material;

class MeshRenderer
{
	typedef unsigned int GLuint;
public:
	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
	MeshRenderer();
	MeshRenderer(const Mesh& mesh, Material* material);
	~MeshRenderer();

	//FUNCTIONS//////////////////////////////////////////////////////////////////////////
	void Render() const;
	void Render(const Matrix4x4& view, const Matrix4x4& projection) const;

	void SetPosition(const Vector3& worldPosition);
	void SetVec3Uniform(const char* uniformName, const Vector3& value);
#pragma todo("Make this private again and set up the accessor functions for all the methods associated")
	Material* m_material;
private:
	Mesh m_mesh;
	GLuint m_vaoID;
	Matrix4x4 m_model;
};