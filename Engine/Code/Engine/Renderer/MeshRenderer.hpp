#pragma once
#include "Engine/Math/Matrix4x4.hpp"
#include "Engine/Renderer/Vertex.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

class Material;
class Mesh;

class MeshRenderer
{
    typedef unsigned int GLuint;
public:
    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    MeshRenderer();
    MeshRenderer(Mesh* mesh, Material* material);
    ~MeshRenderer();

    //FUNCTIONS//////////////////////////////////////////////////////////////////////////
    void Render() const;
    void RotateAround(float degrees, const Vector3& axis);
    void SetPosition(const Vector3& worldPosition);
    void SetVec3Uniform(const char* uniformName, const Vector3& value);
    
    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    Material* m_material;
    Mesh* m_mesh;

private:
    GLuint m_vaoID;
    Matrix4x4 m_model;

    MeshRenderer(const MeshRenderer&);
};