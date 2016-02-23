#pragma once
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Math/Matrix4x4.hpp"

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

private:
	Mesh m_mesh;
	Material* m_material;
	GLuint m_vaoID;
	Matrix4x4 m_model;
};