#pragma once

#include <string>
#include "Engine/Renderer/RGBA.hpp"

class Vector2;
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
	enum DrawMode
	{
		QUADS,
		QUAD_STRIP,
		POINTS,
		LINES,
		LINE_LOOP,
		POLYGON,
		NUM_DRAW_MODES
	};

	TheRenderer();
	void ClearScreen(float red, float green, float blue);
	void ClearScreen(const RGBA& color);
	void SetOrtho(const Vector2& bottomLeft, const Vector2& topRight);
	void SetPerspective(float fovDegreesY, float aspect, float nearDist, float farDist);
	void SetColor(float red, float green, float blue, float alpha);
	void SetColor(const RGBA& color);
	void SetPointSize(float size);
	void SetLineWidth(float width);
	void PushMatrix();
	void PopMatrix();
	void Translate(float x, float y, float z);
	void Translate(const Vector2& xy);
	void Translate(const Vector3& xyz);
	void Rotate(float rotationDegrees);
	void Rotate(float rotationDegrees, float x, float y, float z);
	void Scale(float x, float y, float z);
	void EnableAdditiveBlending();
	void EnableAlphaBlending();
	void EnableInvertedBlending();
	void EnableDepthTest(bool enabled);
	void EnableFaceCulling(bool enabled);
	void BindTexture(const Texture& texture);
	void UnbindTexture();

	int GenerateBufferID();
	void DeleteBuffers(int vboID);
	void BindAndBufferVBOData(int vboID, const Vertex_PCT* vertexes, int numVerts);
	void DrawVertexArray(const Vertex_PCT* vertexes, int numVertexes, DrawMode drawMode = QUADS, Texture* texture = nullptr); 
	void DrawVBO_PCT(unsigned int vboID, int numVerts, DrawMode drawMode = QUADS, Texture* texture = nullptr);

	void DrawPoint(const Vector2& point, const RGBA& color = RGBA::WHITE);
	void DrawPoint(const Vector3& point, const RGBA& color = RGBA::WHITE);
	void DrawPoint(float x, float y, const RGBA& color = RGBA::WHITE);
	void DrawLine(const Vector2& start, const Vector2& end, const RGBA& color = RGBA::WHITE, float lineThickness = 1.0f);
	void DrawLine(const Vector3& start, const Vector3& end, const RGBA& color = RGBA::WHITE, float lineThickness = 1.0f);
	void DrawAABB(const AABB2& bounds, const RGBA& color = RGBA::WHITE);
	void DrawAABB(const AABB3& bounds, const RGBA& color = RGBA::WHITE);
	void DrawAABBBoundingBox(const AABB3& bounds, const RGBA& color = RGBA::WHITE);
	void DrawTexturedAABB(const AABB2& bounds, const Vector2& texCoordMins, const Vector2& texCoordMaxs, Texture* texture = nullptr, const RGBA& color = RGBA::WHITE);
	void DrawTexturedFace(const Face& face, const Vector2& texCoordMins, const Vector2& texCoordMaxs, Texture* texture = nullptr, const RGBA& color = RGBA::WHITE);
	void DrawPolygonOutline(const Vector2& center, float radius, int numSides, float radianOffset, const RGBA& color = RGBA::WHITE);
	void DrawPolygon(const Vector2& center, float radius, int numSides, float radianOffset, const RGBA& color = RGBA::WHITE);
	void DrawText2D(const Vector2& startBottomLeft, const std::string& asciiText, float cellWidth, float cellHeight, const RGBA& tint = RGBA::WHITE, bool drawShadow = false, const BitmapFont* font = nullptr);

	unsigned char GetDrawMode(DrawMode mode);

	static TheRenderer* instance;

	static const int CIRCLE_SIDES = 50;
	static const int HEXAGON_SIDES = 6;

	static const unsigned char plainWhiteTexel[3];

private:
	BitmapFont* m_defaultFont;
	Texture* m_defaultTexture;
};