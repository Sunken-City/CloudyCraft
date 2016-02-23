#pragma once
#include <string>
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"

//-----------------------------------------------------------------------------------------------
#define UNUSED(x) (void)(x);
#define CHECK_RENDERER if (TheRenderer::instance == nullptr) {return;};
#define CHECK_RENDERER_RETURN(x) if (TheRenderer::instance == nullptr) {return x;};

class Vector3;
class AABB2;
class AABB3;
class Texture;
class Face;
class BitmapFont;
struct Vertex_PCT;

class TheRenderer
{
public:
	//ENUMS//////////////////////////////////////////////////////////////////////////
	enum DrawMode
	{
		QUADS,
		QUAD_STRIP,
		POINTS,
		LINES,
		LINE_LOOP,
		POLYGON,
		TRIANGLES,
		NUM_DRAW_MODES
	};

	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
	TheRenderer();
	~TheRenderer();

	//FUNCTIONS//////////////////////////////////////////////////////////////////////////
	void ClearScreen(float red, float green, float blue);
	void ClearScreen(const RGBA& color);
	void PushMatrix();
	void PopMatrix();
	void Translate(float x, float y, float z);
	void Translate(const Vector2& xy);
	void Translate(const Vector3& xyz);
	void Rotate(float rotationDegrees);
	void Rotate(float rotationDegrees, float x, float y, float z);
	void Scale(float x, float y, float z);
	unsigned char GetDrawMode(DrawMode mode);

	//STATE MODIFICATION//////////////////////////////////////////////////////////////////////////
	void EnableAdditiveBlending();
	void EnableAlphaBlending();
	void EnableInvertedBlending();
	void EnableDepthTest(bool enabled);
	void EnableFaceCulling(bool enabled);
	void BindTexture(const Texture& texture);
	void UnbindTexture();
	void SetOrtho(const Vector2& bottomLeft, const Vector2& topRight);
	void SetPerspective(float fovDegreesY, float aspect, float nearDist, float farDist); 
	void SetColor(float red, float green, float blue, float alpha);
	void SetColor(const RGBA& color);
	void SetPointSize(float size);
	void SetLineWidth(float width);

	//BUFFERS//////////////////////////////////////////////////////////////////////////
	int GenerateBufferID();
	void DeleteBuffers(int vboID);
	void BindAndBufferVBOData(int vboID, const Vertex_PCT* vertexes, int numVerts);
	void DrawVertexArray(const Vertex_PCT* vertexes, int numVertexes, DrawMode drawMode = QUADS, Texture* texture = nullptr); 
	void DrawVBO_PCT(unsigned int vboID, int numVerts, DrawMode drawMode = QUADS, Texture* texture = nullptr);

	//DRAWING//////////////////////////////////////////////////////////////////////////
	void DrawPoint(const Vector2& point, const RGBA& color = RGBA::WHITE, float pointSize = 1.0f);
	void DrawPoint(const Vector3& point, const RGBA& color = RGBA::WHITE, float pointSize = 1.0f);
	void DrawPoint(float x, float y, const RGBA& color = RGBA::WHITE, float pointSize = 1.0f);
	void DrawLine(const Vector2& start, const Vector2& end, const RGBA& color = RGBA::WHITE, float lineThickness = 1.0f);
	void DrawLine(const Vector3& start, const Vector3& end, const RGBA& color = RGBA::WHITE, float lineThickness = 1.0f);
	void DrawAABB(const AABB2& bounds, const RGBA& color = RGBA::WHITE);
	void DrawAABB(const AABB3& bounds, const RGBA& color = RGBA::WHITE);
	void DrawAABBBoundingBox(const AABB3& bounds, const RGBA& color = RGBA::WHITE);
	void DrawTexturedAABB3(const AABB3& bounds, const RGBA& color = RGBA::WHITE, const Vector2& texCoordMins = Vector2::ZERO, const Vector2& texCoordMaxs = Vector2::ONE, Texture* texture = nullptr);
	void DrawTexturedAABB(const AABB2& bounds, const Vector2& texCoordMins, const Vector2& texCoordMaxs, Texture* texture = nullptr, const RGBA& color = RGBA::WHITE);
	void DrawTexturedFace(const Face& face, const Vector2& texCoordMins, const Vector2& texCoordMaxs, Texture* texture = nullptr, const RGBA& color = RGBA::WHITE);
	void DrawPolygonOutline(const Vector2& center, float radius, int numSides, float radianOffset, const RGBA& color = RGBA::WHITE);
	void DrawPolygon(const Vector2& center, float radius, int numSides, float radianOffset, const RGBA& color = RGBA::WHITE);
	void DrawText2D(const Vector2& startBottomLeft, const std::string& asciiText, float cellWidth, float cellHeight, const RGBA& tint = RGBA::WHITE, bool drawShadow = false, const BitmapFont* font = nullptr);
	void DrawText2D(const Vector2& position, const std::string& asciiText, float scale, const RGBA& tint = RGBA::WHITE, bool drawShadow = false, const BitmapFont* font = nullptr, const Vector2& right = Vector2::UNIT_X, const Vector2& up = Vector2::UNIT_Y);

	//TEMPORARY SECTION FOR IN-CLASS LECTURE
	//All of these functions were coded as part of a lecture, and the refactor pass is a later lecture.
									typedef unsigned int GLuint;
									typedef int GLint;
									typedef int GLsizei;
									typedef unsigned int GLenum;
									typedef bool GLboolean;
									GLuint LoadShader(const char* filename, GLuint shader_type);
									void ShaderProgramBindProperty(GLuint shaderProgram, const char *name, GLint count, GLenum type, GLboolean normalize, GLsizei stride, GLsizei offset);
									GLuint CreateAndLinkProgram(GLuint vs, GLuint fs);
									void BindMeshToVAO(GLuint vao, GLuint vbo, GLuint ibo, GLuint program);
									GLuint RenderBufferCreate(void* data, size_t count, size_t elementSize, GLenum usage/* = GL_STATIC_DRAW*/);
									void RenderBufferDestroy(GLuint buffer);
									//TEMPORARY FUNCTION FOR LEARNING IN CLASS, WILL GET CLEANED UP
									void DoThing();

								#pragma warning(disable:4201) //nonstandard extension used: nameless struct/union
									struct Vector4
									{
										union {
											float data[4];
											struct {
												float x, y, z, w;
											};
											struct {
												float r, g, b, a;
											};
										};

										Vector4(float v) : x(v), y(v), z(v), w(v) {};
										Vector4(float xv, float yv, float zv) : x(xv), y(yv), z(zv), w(1.0f) {};
										Vector4(float xv, float yv, float zv, float wv) : x(xv), y(yv), z(zv), w(wv) {};
									};

									struct mat4x4
									{
										mat4x4() {};
										union {
											float data[16];
											Vector4 column[4];
										};
										static void MatrixMakeIdentity(mat4x4* matrix)
										{
											const float data[16] = {
												1.0f, 0.0f, 0.0f, 0.0f,
												0.0f, 1.0f, 0.0f, 0.0f,
												0.0f, 0.0f, 1.0f, 0.0f,
												0.0f, 0.0f, 0.0f, 1.0f
											};
											memcpy(matrix->data, data, sizeof(data));
										}
										static void MatrixMakeTranslation(mat4x4* matrix, const Vector3& translate)
										{
											const float data[16] = {
												1.0f, 0.0f, 0.0f, translate.x,
												0.0f, 1.0f, 0.0f, translate.y,
												0.0f, 0.0f, 1.0f, translate.z,
												0.0f, 0.0f, 0.0f, 1.0f
											};
											memcpy(matrix->data, data, sizeof(data));
										}
// 										static void MatrixMakeLookAt(mat4x4* matrix, const Vector3& position, const Vector3& up, const Vector3& target)
// 										{
// 											Vector3 direction = target - position;
// 											direction.Normalize();
// 											Vector3 right = direction.Cross(up);
// 											Vector3 localUp = right.Cross(direction);
// 
// 											//-right-
// 											//-up-
// 											//-dir-
// 
// 											//MatrixSetBasis(mat, right, localUp, dir);
// 											//MatrixSetTranslation(mat, pos);
// 
// 											//not sure if I need this
// 											/*const float data[16] = {
// 												1.0f, 0.0f, 0.0f, translate.x,
// 												0.0f, 1.0f, 0.0f, translate.y,
// 												0.0f, 0.0f, 1.0f, translate.z,
// 												0.0f, 0.0f, 0.0f, 1.0f
// 											};
// 											memcpy(matrix->data, data, sizeof(data));*/
// 										}
// 										static void MatrixInvertOrthogonal(mat4x4* matrix)
// 										{
// 											std::swap(matrix[1], matrix[4]);
// 											std::swap(matrix[2], matrix[8]);
// 											std::swap(matrix[6], matrix[9]);
// 
// 											MatrixSetTranslation(mat, Vector3(-matrix[12], -matrix[13], -matrix[14]);
// 										}
// 										static void MatrixMakeRotationEuler(mat4x4* matrix)
// 										{
// 											float sx = sin(pitch);
// 											float cx = cos(pitch);
// 
// 											float sy = sin(yaw);
// 											float cy = cos(yaw);
// 
// 											float sz = sin(roll);
// 											float cz = cos(roll);
// 
// 											float const values[] = {
// 												cy*cz + sx*sy*sz, cy*sz - cz*sx*sy, cx*sy, offset.x,
// 												-cx*sz, cx*cz, sx, offset.y,
// 												-cz*sy + cy*sx*sz, -sy*sz - cy*cz*sx, cx*cy, offset.z,
// 												0.0f, 0.0f, 0.0f, 1.0f
// 											};
// 										}
									};
									bool ShaderProgramSetVec4(GLuint shader, const char* name, const Vector4 &v);
									bool ShaderProgramSetMat4(GLuint shader, const char* name, const mat4x4 &v);
									bool ShaderProgramSetInt(GLuint shader, const char* name, int val);
									void MatrixMakeProjectionOrthogonal(mat4x4* mat, float width, float height, float nz, float fz);
									void MatrixMakePerspective(mat4x4* mat, float width, float height, float nz, float fz);
									int CreateSampler(GLenum min_filter, GLenum magFilter, GLenum uWrap, GLenum vWrap);
	//END TEMPORARY SECTION FOR CLASS
							
	//CONSTANTS//////////////////////////////////////////////////////////////////////////
	static const int CIRCLE_SIDES = 50;
	static const int HEXAGON_SIDES = 6;
	static const unsigned char plainWhiteTexel[3];

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	static TheRenderer* instance;
private:
	BitmapFont* m_defaultFont;
	Texture* m_defaultTexture;
};