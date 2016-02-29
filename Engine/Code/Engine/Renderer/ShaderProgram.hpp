#pragma once
#include <string>
#include <vector>

class Vector3;
class Vector4;
class Matrix4x4;

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct Uniform
{
	//ENUMS//////////////////////////////////////////////////////////////////////////
	enum class DataType
	{
		MATRIX_4X4,
		VECTOR3,
		VECTOR4,
		FLOAT,
		INT,
		NUM_TYPES
	};

	std::string name;
	DataType type;
	unsigned int bindPoint;
	unsigned int size;
	unsigned int textureIndex;
};

//----------------------------------------------------------------------------------------------------------------------------------------------------------
class ShaderProgram
{
public:
	//Typedef'd so that we don't need to include Windows.h
	typedef unsigned int GLuint;
	typedef int GLint;
	typedef int GLsizei;
	typedef unsigned int GLenum;
	typedef bool GLboolean;

	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
	ShaderProgram();
	ShaderProgram(const char* vertShaderPath, const char* fragShaderPath);
	~ShaderProgram();

	//FUNCTIONS//////////////////////////////////////////////////////////////////////////
	GLuint LoadShader(const char* filename, GLuint shader_type);
	GLuint CreateAndLinkProgram(GLuint vs, GLuint fs);
	void ShaderProgramBindProperty(const char *name, GLint count, GLenum type, GLboolean normalize, GLsizei stride, GLsizei offset);
	void FindAllUniforms();
	bool SetUniform(const char* name, void* value);
	bool SetVec3Uniform(const char* name, const Vector3 &value);
	bool SetVec3Uniform(const char *name, const Vector3 &value, unsigned int arrayIndex);
	bool SetVec4Uniform(const char* name, const Vector4 &value);
	bool SetVec4Uniform(const char *name, const Vector4 &value, unsigned int arrayIndex);
	bool SetMatrix4x4Uniform(const char* name, const Matrix4x4 &value);
	bool SetIntUniform(const char* name, int value);
	bool SetIntUniform(const char* name, int value, unsigned int arrayIndex);
	bool SetFloatUniform(const char* name, float value);
	bool SetFloatUniform(const char* name, float value, unsigned int arrayIndex);

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	GLuint m_vertexShaderID;
	GLuint m_fragmentShaderID;
	GLuint m_shaderProgramID;
	std::vector<Uniform> m_uniforms;
};