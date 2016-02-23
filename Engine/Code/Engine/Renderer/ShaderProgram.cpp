#include "Engine/Renderer/ShaderProgram.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <stdlib.h>
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Input/InputOutputUtils.hpp"
#include "Engine/Renderer/OpenGLExtensions.hpp"

//-----------------------------------------------------------------------------------
ShaderProgram::ShaderProgram()
	: m_vertexShaderID(0)
	, m_fragmentShaderID(0)
	, m_shaderProgramID(0)
{

}

//-----------------------------------------------------------------------------------
ShaderProgram::ShaderProgram(const char* vertShaderPath, const char* fragShaderPath)
	: m_vertexShaderID(LoadShader(vertShaderPath, GL_VERTEX_SHADER))
	, m_fragmentShaderID(LoadShader(fragShaderPath, GL_FRAGMENT_SHADER))
	, m_shaderProgramID(CreateAndLinkProgram(m_vertexShaderID, m_fragmentShaderID))
{
	ASSERT_OR_DIE(m_vertexShaderID != NULL && m_fragmentShaderID != NULL, "Error: Vertex or Fragment Shader was null");
	ASSERT_OR_DIE(m_shaderProgramID != NULL, "Error: Program linking id was null");
	FindAllUniforms();
}

//-----------------------------------------------------------------------------------
ShaderProgram::~ShaderProgram()
{
	glDeleteShader(m_vertexShaderID);
	glDeleteShader(m_fragmentShaderID);
	glDeleteProgram(m_shaderProgramID);
}

//-----------------------------------------------------------------------------------
GLuint ShaderProgram::LoadShader(const char* filename, GLenum shader_type)
{
	char* fileBuffer = FileReadIntoNewBuffer(filename);

	GLuint shader_id = glCreateShader(shader_type);
	ASSERT_OR_DIE(shader_id != NULL, "Failed to create shader");

	GLint src_length = strlen(fileBuffer);
	glShaderSource(shader_id, 1, &fileBuffer, &src_length);

	glCompileShader(shader_id);

	GLint status;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
	if (GL_FALSE == status)
	{
		GLint logLength;
		glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &logLength);

		char *buffer = new char[logLength + 1];
		glGetShaderInfoLog(shader_id, logLength, &logLength, buffer);

		buffer[logLength] = '\0'; //or NULL
		glDeleteShader(shader_id);
		std::string bufferString(buffer);

		//Get the full path
		char filePath[_MAX_PATH];
		_fullpath(filePath, filename, _MAX_PATH);

		std::size_t firstSemicolon = bufferString.find(":");
		std::size_t secondSemicolon = bufferString.find(":", firstSemicolon + 1, 1);
		std::string formattedErrorString = bufferString.substr(secondSemicolon + 1);
		
		std::vector<std::string>* inParenthesis = ExtractStringsBetween(bufferString, "(", ")");
		int lineNumber = std::stoi(inParenthesis->at(0));
		delete inParenthesis;
		DebuggerPrintf("%s(%d): %s", filePath, lineNumber, formattedErrorString.c_str());

		const char* glVersion = (const char*)glGetString(GL_VERSION);
		const char* glslVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
		ERROR_AND_DIE(Stringf("%s\nIn file: %s\nOn line: %i \n\n %s \nOpenGL version: %s\nGLSL version: %s", formattedErrorString.c_str(), filename, lineNumber, buffer, glVersion, glslVersion));
	}

	//Todo: print errors if failed

	free(fileBuffer);
	fileBuffer = nullptr;
	return shader_id;
}

//-----------------------------------------------------------------------------------
GLuint ShaderProgram::CreateAndLinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
	GLuint program_id = glCreateProgram();
	ASSERT_OR_DIE(program_id != NULL, "Failed to create shader program");

	glAttachShader(program_id, vertexShader);
	glAttachShader(program_id, fragmentShader);

	glLinkProgram(program_id);

	GLint status;
	//Get iv gets a 'v'ariable amount of 'i'ntegers
	glGetProgramiv(program_id, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint logLength;
		glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &logLength);

		char *buffer = new char[logLength + 1];
		glGetProgramInfoLog(program_id, logLength, &logLength, buffer);

		buffer[logLength] = '\0'; //or NULL
		glDeleteShader(program_id);
		std::string bufferString(buffer);

		std::size_t firstSemicolon = bufferString.find(":");
		std::size_t secondSemicolon = bufferString.find(":", firstSemicolon + 1, 1);
		std::string formattedErrorString = bufferString.substr(secondSemicolon + 1);

		std::vector<std::string>* inParenthesis = ExtractStringsBetween(bufferString, "(", ")");
		int lineNumber = std::stoi(inParenthesis->at(0));
		delete inParenthesis;
		DebuggerPrintf("(%d): %s", lineNumber, formattedErrorString.c_str());

		const char* glVersion = (const char*)glGetString(GL_VERSION);
		const char* glslVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
		ERROR_AND_DIE(Stringf("%s\nOn line: %i \n\n %s \nOpenGL version: %s\nGLSL version: %s", formattedErrorString.c_str(), lineNumber, buffer, glVersion, glslVersion));
	}
	else
	{
		//Let OpenGL clean up video memory for the shaders
		glDetachShader(program_id, vertexShader);
		glDetachShader(program_id, fragmentShader);
	}

	return program_id;
}

//-----------------------------------------------------------------------------------
void ShaderProgram::ShaderProgramBindProperty(const char *name, GLint count, GLenum type, GLboolean normalize, GLsizei stride, GLsizei offset)
{
	GLint property = glGetAttribLocation(m_shaderProgramID, name);
	if (property >= 0)
	{
		glEnableVertexAttribArray(property);
		glVertexAttribPointer(property, //Bind point to shader
			count, //Number of data elements passed
			type, //Type of Data
			normalize, //normalize the data for us
			stride, //stride
			(GLvoid*)offset //From that point in memory, how far do we have to go to get the value?
			);
	}
}

//-----------------------------------------------------------------------------------
void ShaderProgram::FindAllUniforms()
{
	GLint numberOfActiveUniforms;
	glGetProgramiv(m_shaderProgramID, GL_ACTIVE_UNIFORMS, &numberOfActiveUniforms);
	GLint maxNameLength;
	glGetProgramiv(m_shaderProgramID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);
	for (unsigned int index = 0; index < numberOfActiveUniforms; index++)
	{
		Uniform uniform;
		GLint size;
		GLenum type;
		char* nameBuffer = new char[maxNameLength];
		glGetActiveUniform(m_shaderProgramID, index, maxNameLength, NULL, &size, &type, nameBuffer);
		uniform.name = std::string(nameBuffer);
		switch (type)
		{
		case GL_FLOAT_MAT4:
			uniform.type = Uniform::DataType::MATRIX_4X4;
			break;
		case GL_FLOAT_VEC4:
			uniform.type = Uniform::DataType::VECTOR4;
			break;
		case GL_FLOAT_VEC3:
			uniform.type = Uniform::DataType::VECTOR3;
			break;
		case GL_FLOAT:
			uniform.type = Uniform::DataType::FLOAT;
			break;
		case GL_INT:
			uniform.type = Uniform::DataType::INT;
			break;
		default:
			break;
		}
		uniform.size = size;
		uniform.bindPoint = index;
		uniform.textureIndex = 0;
		m_uniforms.push_back(uniform);
		delete nameBuffer;
	}
}

//-----------------------------------------------------------------------------------
bool ShaderProgram::SetUniform(const char* name, void* value)
{
	std::string inputName(name);
	Uniform matchingUniform;
	bool wasFound = false;
	for (Uniform uniform : m_uniforms)
	{
		if (inputName == uniform.name)
		{
			matchingUniform = uniform;
			wasFound = true;
			break;
		}
	}
	if (!wasFound)
	{
		return false;
	}
	switch (matchingUniform.type)
	{
	case Uniform::DataType::MATRIX_4X4:
		return SetMatrix4x4Uniform(name, *static_cast<Matrix4x4*>(value));
	case Uniform::DataType::VECTOR4:
		return SetVec4Uniform(name, *static_cast<Vector4*>(value));
	case Uniform::DataType::VECTOR3:
		return SetVec3Uniform(name, *static_cast<Vector3*>(value));
	case Uniform::DataType::FLOAT:
		return SetFloatUniform(name, *static_cast<float*>(value));
	case Uniform::DataType::INT:
		return SetIntUniform(name, *static_cast<int*>(value));
	default:
		break;
	}
}

//-----------------------------------------------------------------------------------
bool ShaderProgram::SetVec3Uniform(const char *name, const Vector3 &value)
{
	glUseProgram(m_shaderProgramID);
	#pragma TODO("THIS IS A PROBLEM. This is what we needed to get rid of. Don't call this from other functions!")
	GLint loc = glGetUniformLocation(m_shaderProgramID, name);
	if (loc >= 0)
	{
		glUniform3fv(loc, 1, (GLfloat*)&value);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------------
bool ShaderProgram::SetVec4Uniform(const char *name, const Vector4 &value)
{
	glUseProgram(m_shaderProgramID);
	GLint loc = glGetUniformLocation(m_shaderProgramID, name);
	if (loc >= 0)
	{
		glUniform4fv(loc, 1, (GLfloat*)&value);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------------
bool ShaderProgram::SetMatrix4x4Uniform(const char* name, const Matrix4x4 &value)
{
	glUseProgram(m_shaderProgramID);
	GLint loc = glGetUniformLocation(m_shaderProgramID, name);
	if (loc >= 0)
	{
		glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat*)&value); //location, number of elements, do you want gl to transpose matrix?, matrix
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------------
bool ShaderProgram::SetIntUniform(const char* name, int value)
{
	glUseProgram(m_shaderProgramID);
	GLint loc = glGetUniformLocation(m_shaderProgramID, name);
	if (loc >= 0)
	{
		glUniform1iv(loc, 1, (GLint*)&value); //location, number of elements, do you want gl to transpose matrix?, matrix
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------------
bool ShaderProgram::SetFloatUniform(const char* name, float value)
{
	glUseProgram(m_shaderProgramID);
	GLint loc = glGetUniformLocation(m_shaderProgramID, name);
	if (loc >= 0)
	{
		glUniform1fv(loc, 1, (GLfloat*)&value); //location, number of elements, do you want gl to transpose matrix?, matrix
		return true;
	}
	return false;
}