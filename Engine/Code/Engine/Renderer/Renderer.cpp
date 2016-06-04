#include "Engine/Renderer/Renderer.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <math.h>

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Renderer/AABB3.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Vertex.hpp"
#include "Engine/Renderer/Face.hpp"
#include "Engine/Renderer/OpenGLExtensions.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Framebuffer.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Input/InputOutputUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/BuildConfig.hpp"
#include "Engine/Core/Memory/MemoryTracking.hpp"
#include "Engine/Time/Time.hpp"

#pragma comment( lib, "opengl32" ) // Link in the OpenGL32.lib static library
#pragma comment( lib, "Glu32" ) // Link in the Glu32.lib static library

Renderer* Renderer::instance = nullptr;

const unsigned char Renderer::plainWhiteTexel[3] = { 255, 255, 255 };

//TEMP
static GLuint gVBO = NULL;
static GLuint gVAO = NULL;
static GLuint gIBO = NULL;
static GLuint gSampler = NULL;
static GLuint gDiffuseTex = NULL;

//-----------------------------------------------------------------------------------
Renderer::Renderer()
    : m_defaultTexture(Texture::CreateTextureFromData("PlainWhite", const_cast<uchar*>(plainWhiteTexel), 3, Vector2Int::ONE))
    , m_fbo(nullptr)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    hookUpOpenGLPointers();
    m_defaultFont = BitmapFont::CreateOrGetFont("SquirrelFixedFont");
    m_defaultShader = new ShaderProgram("Data/Shaders/fixedVertexFormat.vert", "Data/Shaders/fixedVertexFormat.frag");
    m_defaultMaterial = new Material(m_defaultShader, RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::RENDER_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND));
    m_defaultMaterial->SetDiffuseTexture(m_defaultTexture);
}

//-----------------------------------------------------------------------------------
Renderer::~Renderer()
{
    delete m_defaultShader;
    delete m_defaultMaterial;
}

//-----------------------------------------------------------------------------------
void Renderer::ClearScreen(float red, float green, float blue)
{
    glClearColor(red, green, blue, 1.f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

//-----------------------------------------------------------------------------------
void Renderer::ClearScreen(const RGBA& color)
{
    glClearColor(color.red / 255.0f, color.green / 255.0f, color.blue / 255.0f, 1.f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

//-----------------------------------------------------------------------------------
void Renderer::ClearColor(const RGBA& color)
{
    glClearColor(color.red / 255.0f, color.green / 255.0f, color.blue / 255.0f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
}

//-----------------------------------------------------------------------------------
void Renderer::SetOrtho(const Vector2& bottomLeft, const Vector2& topRight)
{
    glLoadIdentity();
    glOrtho(bottomLeft.x, topRight.x, bottomLeft.y, topRight.y, 0.f, 1.f);
}

//-----------------------------------------------------------------------------------
void Renderer::BeginOrtho(const Vector2& bottomLeft, const Vector2& topRight)
{
    Matrix4x4 translation = Matrix4x4::IDENTITY;
    Matrix4x4 projection = Matrix4x4::IDENTITY;
    float width = topRight.x - bottomLeft.x;
    float height = topRight.y - bottomLeft.y;

    Matrix4x4::MatrixMakeProjectionOrthogonal(&projection, width, height, 0.f, 1.f);
    Matrix4x4::MatrixMakeTranslation(&translation, Vector3(-width / 2.0f, -height / 2.0f, 0.0f));
    m_projStack.PushWithoutMultiply(projection);
    m_projStack.Push(translation);
}

//-----------------------------------------------------------------------------------
void Renderer::EndOrtho()
{
    //Pop the translation
    m_projStack.Pop();
    //Pop the projection matrix
    m_projStack.Pop();
}

//-----------------------------------------------------------------------------------
void Renderer::SetPerspective(float fovDegreesY, float aspect, float nearDist, float farDist)
{
    glLoadIdentity();
    gluPerspective(fovDegreesY, aspect, nearDist, farDist); //Note: absent in OpenGL ES 2.0
}

//-----------------------------------------------------------------------------------
void Renderer::BeginPerspective(float fovDegreesY, float aspect, float nearDist, float farDist)
{
    Matrix4x4 proj = Matrix4x4::IDENTITY;
    Matrix4x4::MatrixMakePerspective(&proj, fovDegreesY, aspect, nearDist, farDist);
    PushProjection(proj);
}

//-----------------------------------------------------------------------------------
void Renderer::EndPerspective()
{
    PopProjection();
}

//-----------------------------------------------------------------------------------
void Renderer::RotateView(float degrees, const Vector3& axis)
{
    //From page 111 in Chapter 8 of 3D Math Primer for Graphics and Game Development
    Matrix4x4 rotation = Matrix4x4::IDENTITY;
    rotation.Rotate(degrees, axis);
    PushView(rotation);
}

//-----------------------------------------------------------------------------------
void Renderer::TranslateView(const Vector3& translation)
{
    Matrix4x4 translationMatrix = Matrix4x4::IDENTITY;
    Matrix4x4::MatrixMakeTranslation(&translationMatrix, translation);
    PushView(translationMatrix);
}

//-----------------------------------------------------------------------------------
void Renderer::DrawPoint(float x, float y, const RGBA& color /*= RGBA::WHITE*/, float pointSize /*= 1.0f*/)
{
    glPointSize(pointSize);
    Vertex_PCT vertex;
    vertex.pos = Vector3(x, y, 0.0f);
    vertex.color = color;
    DrawVertexArray(&vertex, 1, DrawMode::POINTS);
}

//-----------------------------------------------------------------------------------
void Renderer::DrawPoint(const Vector2& point, const RGBA& color /*= RGBA::WHITE*/, float pointSize /*= 1.0f*/)
{
    DrawPoint(point.x, point.y, color, pointSize);
}

//-----------------------------------------------------------------------------------
void Renderer::DrawPoint(const Vector3& point, const RGBA& color /*= RGBA::WHITE*/, float pointSize /*= 1.0f*/)
{
    glPointSize(pointSize);
    Vertex_PCT vertex;
    vertex.pos = point;
    vertex.color = color;
    DrawVertexArray(&vertex, 1, DrawMode::POINTS);
}

//-----------------------------------------------------------------------------------
void Renderer::DrawLine(const Vector2& start, const Vector2& end, const RGBA& color/* = RGBA::WHITE*/, float lineThickness /*= 1.0f*/)
{
    glLineWidth(lineThickness);
    Vertex_PCT vertexes[2];
    vertexes[0].pos = start;
    vertexes[1].pos = end;
    vertexes[0].color = color;
    vertexes[1].color = color;
    DrawVertexArray(vertexes, 2, DrawMode::LINES);
}

//-----------------------------------------------------------------------------------
void Renderer::DrawLine(const Vector3& start, const Vector3& end, const RGBA& color /*= RGBA::WHITE*/, float lineThickness /*= 1.0f*/)
{
    glLineWidth(lineThickness);
    Vertex_PCT vertexes[2];
    vertexes[0].pos = start;
    vertexes[1].pos = end;
    vertexes[0].color = color;
    vertexes[1].color = color;
    DrawVertexArray(vertexes, 2, DrawMode::LINES);
}

//-----------------------------------------------------------------------------------
void Renderer::SetColor(float red, float green, float blue, float alpha)
{
    glColor4f(red, green, blue, alpha);
}

//-----------------------------------------------------------------------------------
void Renderer::SetColor(const RGBA& color)
{
    glColor4ub(color.red, color.green, color.blue, color.alpha);
}

//-----------------------------------------------------------------------------------
void Renderer::SetPointSize(float size)
{
    glPointSize(size);
}

//-----------------------------------------------------------------------------------
void Renderer::SetLineWidth(float width)
{
    glLineWidth(width);
}

//-----------------------------------------------------------------------------------
void Renderer::PushMatrix()
{
    glPushMatrix();
}

//-----------------------------------------------------------------------------------
void Renderer::PopMatrix()
{
    glPopMatrix();
}

//-----------------------------------------------------------------------------------
void Renderer::Translate(float x, float y, float z)
{
    glTranslatef(x, y, z);
}

//-----------------------------------------------------------------------------------
void Renderer::Translate(const Vector2& xy)
{
    glTranslatef(xy.x, xy.y, 0.0f);
}

//-----------------------------------------------------------------------------------
void Renderer::Translate(const Vector3& xyz)
{
    glTranslatef(xyz.x, xyz.y, xyz.z);
}

//-----------------------------------------------------------------------------------
void Renderer::Rotate(float rotationDegrees)
{
    glRotatef(rotationDegrees, 0.f, 0.f, 1.f);
}

//-----------------------------------------------------------------------------------
void Renderer::Rotate(float rotationDegrees, float x, float y, float z)
{
    glRotatef(rotationDegrees, x, y, z);
}

//-----------------------------------------------------------------------------------
void Renderer::Scale(float x, float y, float z)
{
    glScalef(x, y, z);
}

//-----------------------------------------------------------------------------------
void Renderer::EnableAdditiveBlending()
{
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

//-----------------------------------------------------------------------------------
void Renderer::EnableAlphaBlending()
{
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

//-----------------------------------------------------------------------------------
void Renderer::EnableInvertedBlending()
{
    glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
}

//-----------------------------------------------------------------------------------
void Renderer::EnableDepthTest(bool usingDepthTest)
{
    usingDepthTest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
}

//-----------------------------------------------------------------------------------
void Renderer::EnableDepthWrite()
{
    glDepthMask(true);
}

//-----------------------------------------------------------------------------------
void Renderer::DisableDepthWrite()
{
    glDepthMask(false);
}

//-----------------------------------------------------------------------------------
void Renderer::BindTexture(const Texture& texture)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture.m_openglTextureID);
}

//-----------------------------------------------------------------------------------
void Renderer::UnbindTexture()
{
    glBindTexture(GL_TEXTURE_2D, NULL);
    glDisable(GL_TEXTURE_2D);
}

//-----------------------------------------------------------------------------------
int Renderer::GenerateBufferID()
{
    GLuint vboID = 0;
    glGenBuffers(1, &vboID);
    #if defined(TRACK_MEMORY)
        g_memoryAnalytics.TrackRenderBufferAllocation();
    #endif
    return vboID;
}

//-----------------------------------------------------------------------------------
void Renderer::DeleteBuffers(int vboID)
{
    const GLuint id = ((GLuint)vboID);
    glDeleteBuffers(1, &id);
    #if defined(TRACK_MEMORY)
        g_memoryAnalytics.TrackRenderBufferFree();
    #endif
}

//-----------------------------------------------------------------------------------
void Renderer::BindAndBufferVBOData(int vboID, const Vertex_PCT* vertexes, int numVerts)
{
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex_PCT) * numVerts, vertexes, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, NULL);
}

//-----------------------------------------------------------------------------------
void Renderer::BindAndBufferVBOData(int vboID, const Vertex_PCUTB* vertexes, int numVerts)
{
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex_PCUTB) * numVerts, vertexes, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, NULL);
}

//-----------------------------------------------------------------------------------
void Renderer::DrawVertexArray(const Vertex_PCT* vertexes, int numVertexes, DrawMode drawMode /*= DrawMode::QUADS*/)
{
    if (numVertexes == 0)
    {
        return;
    }
    MeshBuilder builder;
    builder.Begin();
    for (int i = 0; i < numVertexes; ++i)
    {
        builder.SetColor(vertexes[i].color);
        builder.SetUV(vertexes[i].texCoords);
        builder.SetTBN(Vector3::ZERO, Vector3::ZERO, Vector3::ZERO);
        builder.AddVertex(vertexes[i].pos);
        builder.AddIndex(i);
    }
    builder.End();

    Mesh* mesh = new Mesh();
    builder.CopyToMesh(mesh, &Vertex_PCUTB::Copy, sizeof(Vertex_PCUTB), &Vertex_PCUTB::BindMeshToVAO);
    mesh->m_drawMode = drawMode;
    MeshRenderer* thingToRender = new MeshRenderer(mesh, m_defaultMaterial);
    m_defaultMaterial->SetMatrices(Matrix4x4::IDENTITY, m_viewStack.GetTop(), m_projStack.GetTop());
    GL_CHECK_ERROR();
    thingToRender->Render();
    delete mesh;
    delete thingToRender;
}

//-----------------------------------------------------------------------------------
void Renderer::DrawVBO_PCT(unsigned int vboID, int numVerts, DrawMode drawMode /*= QUADS*/, Texture* texture /*= nullptr*/)
{
    if (!texture)
    {
        texture = m_defaultTexture;
    }
    BindTexture(*texture);
    glBindBuffer(GL_ARRAY_BUFFER, vboID);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, sizeof(Vertex_PCT), (const GLvoid*)offsetof(Vertex_PCT, pos));
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex_PCT), (const GLvoid*)offsetof(Vertex_PCT, color));
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex_PCT), (const GLvoid*)offsetof(Vertex_PCT, texCoords));

    glDrawArrays(GetDrawMode(drawMode), 0, numVerts);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, NULL);
    UnbindTexture();
}

void Renderer::DrawVBO_PCUTB(unsigned int vboID, int numVerts, DrawMode drawMode /*= QUADS*/, Texture* texture /*= nullptr*/)
{
    if (!texture)
    {
        texture = m_defaultTexture;
    }
    BindTexture(*texture);
    glBindBuffer(GL_ARRAY_BUFFER, vboID);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, sizeof(Vertex_PCUTB), (const GLvoid*)offsetof(Vertex_PCUTB, pos));
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex_PCUTB), (const GLvoid*)offsetof(Vertex_PCUTB, color));
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex_PCUTB), (const GLvoid*)offsetof(Vertex_PCUTB, texCoords));
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex_PCUTB), (const GLvoid*)offsetof(Vertex_PCUTB, tangent));
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex_PCUTB), (const GLvoid*)offsetof(Vertex_PCUTB, bitangent));

    glDrawArrays(GetDrawMode(drawMode), 0, numVerts);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    UnbindTexture();
}

//-----------------------------------------------------------------------------------
void Renderer::DrawText2D
    ( const Vector2& startBottomLeft
    , const std::string& asciiText
    , float cellWidth
    , float cellHeight
    , const RGBA& tint /*= RGBA::WHITE*/
    , bool drawShadow /*= false*/
    , const BitmapFont* font /*= nullptr*/)
{
    const float SHADOW_WIDTH_OFFSET = cellWidth / 10.0f;
    const float SHADOW_HEIGHT_OFFSET = cellHeight / -10.0f;
    const Vector2 SHADOW_OFFSET = Vector2(SHADOW_WIDTH_OFFSET, SHADOW_HEIGHT_OFFSET);

    if (asciiText.empty())
    {
        return;
    }
    MeshBuilder builder;
    builder.Begin();
    if (font == nullptr)
    {
        font = m_defaultFont;
    }
    int stringLength = asciiText.size();
    Vector2 currentPosition = startBottomLeft;
    for (int i = 0; i < stringLength; i++)
    {
        unsigned char currentCharacter = asciiText[i];
        Vector2 topRight = currentPosition + Vector2(cellWidth, cellHeight);
        AABB2 quadBounds = AABB2(currentPosition, topRight);
        AABB2 glyphBounds =	font->GetTexCoordsForGlyph(currentCharacter);
        if (drawShadow)
        {
            AABB2 shadowBounds = AABB2(currentPosition + SHADOW_OFFSET, topRight + SHADOW_OFFSET);
            builder.AddTexturedAABB(shadowBounds, glyphBounds.mins, glyphBounds.maxs, RGBA::BLACK);
        }
        builder.AddTexturedAABB(quadBounds, glyphBounds.mins, glyphBounds.maxs, tint);
        currentPosition.x += cellWidth;
    }
    builder.End();

    Mesh* mesh = new Mesh();
    builder.CopyToMesh(mesh, &Vertex_PCUTB::Copy, sizeof(Vertex_PCUTB), &Vertex_PCUTB::BindMeshToVAO);
    mesh->m_drawMode = DrawMode::TRIANGLES;
    MeshRenderer* thingToRender = new MeshRenderer(mesh, font->GetMaterial());
    m_defaultMaterial->SetMatrices(Matrix4x4::IDENTITY, m_viewStack.GetTop(), m_projStack.GetTop());
    GL_CHECK_ERROR();
    thingToRender->Render();
    delete mesh;
    delete thingToRender;
}

//-----------------------------------------------------------------------------------
void Renderer::DrawText2D(const Vector2& position, const std::string& asciiText, float scale, const RGBA& tint /*= RGBA::WHITE*/, bool drawShadow /*= false*/, const BitmapFont* font /*= nullptr*/, const Vector2& right /*= Vector3::UNIT_X*/, const Vector2& up /*= Vector3::UNIT_Z*/)
{
    //To be used when I expand this method to 3D text
    UNUSED(up);
    UNUSED(right);
    if (asciiText.empty())
    {
        return;
    }
    if (font == nullptr)
    {
        font = m_defaultFont;
    }
    int stringLength = asciiText.size();
    Vector2 cursorPosition = position + (Vector2::UNIT_Y * (float)font->m_maxHeight * scale);
    const Glyph* previousGlyph = nullptr;
    MeshBuilder builder;
    builder.Begin();
    for (int i = 0; i < stringLength; i++)
    {
        unsigned char currentCharacter = asciiText[i];
        const Glyph* glyph = font->GetGlyph(currentCharacter);
        float glyphWidth = static_cast<float>(glyph->width) * scale;
        float glyphHeight = static_cast<float>(glyph->height) * scale;

        if (previousGlyph)
        {
            const Vector2 kerning = font->GetKerning(*previousGlyph, *glyph);
            cursorPosition += (kerning * scale);
        }
        Vector2 offset = Vector2(glyph->xOffset * scale, -glyph->yOffset * scale);
        Vector2 topRight = cursorPosition + offset + Vector2(glyphWidth, 0.0f);
        Vector2 bottomLeft = cursorPosition + offset - Vector2(0.0f, glyphHeight);
        AABB2 quadBounds = AABB2(bottomLeft, topRight);
        AABB2 glyphBounds = font->GetTexCoordsForGlyph(*glyph);
        if (drawShadow)
        {
            float shadowWidthOffset = glyphWidth / 10.0f;
            float shadowHeightOffset = glyphHeight / -10.0f;
            Vector2 shadowOffset = Vector2(shadowWidthOffset, shadowHeightOffset);
            AABB2 shadowBounds = AABB2(bottomLeft + shadowOffset, topRight + shadowOffset);
            builder.AddTexturedAABB(shadowBounds, glyphBounds.mins, glyphBounds.maxs, RGBA::BLACK);
        }
        builder.AddTexturedAABB(quadBounds, glyphBounds.mins, glyphBounds.maxs, tint);
        cursorPosition.x += glyph->xAdvance * scale;
        previousGlyph = glyph;
    }
    builder.End();

    Mesh* mesh = new Mesh();
    builder.CopyToMesh(mesh, &Vertex_PCUTB::Copy, sizeof(Vertex_PCUTB), &Vertex_PCUTB::BindMeshToVAO);
    mesh->m_drawMode = DrawMode::TRIANGLES;
    MeshRenderer* thingToRender = new MeshRenderer(mesh, font->GetMaterial());
    m_defaultMaterial->SetMatrices(Matrix4x4::IDENTITY, m_viewStack.GetTop(), m_projStack.GetTop());
    GL_CHECK_ERROR();
    thingToRender->Render();
    delete mesh;
    delete thingToRender;
}

//-----------------------------------------------------------------------------------
unsigned char Renderer::GetDrawMode(DrawMode mode) const
{
    switch (mode)
    {
    case DrawMode::QUADS:
        return GL_QUADS;
    case DrawMode::QUAD_STRIP:
        return GL_QUAD_STRIP;
    case DrawMode::POINTS:
        return GL_POINTS;
    case DrawMode::LINES:
        return GL_LINES;
    case DrawMode::LINE_LOOP:
        return GL_LINE_LOOP;
    case DrawMode::POLYGON:
        return GL_POLYGON;
    case DrawMode::TRIANGLES:
        return GL_TRIANGLES;
    default:
        return GL_POINTS;
    }
}

//-----------------------------------------------------------------------------------
GLuint Renderer::GenerateVAOHandle()
{
    GLuint vaoID;
    glGenVertexArrays(1, &vaoID);
    ASSERT_OR_DIE(vaoID != NULL, "VAO was null");
    #if defined(TRACK_MEMORY)
        g_memoryAnalytics.TrackVAOAllocation();
    #endif
    return vaoID;
}

//-----------------------------------------------------------------------------------
void Renderer::DeleteVAOHandle(GLuint vaoID)
{
    glDeleteVertexArrays(1, &vaoID);
    #if defined(TRACK_MEMORY)
        g_memoryAnalytics.TrackVAOFree();
    #endif
}

//-----------------------------------------------------------------------------------
void Renderer::ClearDepth(float depthValue)
{
    glClearDepth(depthValue);
    glClear(GL_DEPTH_BUFFER_BIT);
}

//-----------------------------------------------------------------------------------
void Renderer::UnbindIbo()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
}

//-----------------------------------------------------------------------------------
GLuint Renderer::RenderBufferCreate(void* data, size_t count, size_t elementSize, GLenum usage/* = GL_STATIC_DRAW*/)
{
    GLuint buffer;
    glGenBuffers(1, &buffer);

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, count * elementSize, data, usage);
    glBindBuffer(GL_ARRAY_BUFFER, NULL);

    #if defined(TRACK_MEMORY)
        g_memoryAnalytics.TrackRenderBufferAllocation();
    #endif

    return buffer;
}

//-----------------------------------------------------------------------------------
void Renderer::RenderBufferDestroy(GLuint buffer)
{
    glDeleteBuffers(1, &buffer);

    #if defined(TRACK_MEMORY)
        g_memoryAnalytics.TrackRenderBufferFree();
    #endif
}

//-----------------------------------------------------------------------------------
int Renderer::CreateSampler(GLenum min_filter, //fragment counts for more than one texel, how does it shrink?
    GLenum magFilter, //more texels than fragments, how does it stretch?
    GLenum uWrap, //If u is < 0 or > 1, how does it behave?
    GLenum vWrap) //Same, but for v
{
    GLuint id;
    glGenSamplers(1, &id);

    glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, min_filter);
    glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, magFilter);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_S, uWrap); //For some reason, OpenGL refers to UV's as ST's
    glSamplerParameteri(id, GL_TEXTURE_WRAP_T, vWrap);

    return id;
}

//-----------------------------------------------------------------------------------
void Renderer::DeleteSampler(GLuint id)
{
    glDeleteSamplers(1, &id);
}

void Renderer::GLCheckError(const char* file, size_t line)
{
#ifdef CHECK_GL_ERRORS
    GLenum error = glGetError();
    if (error != 0)
    {
        const char* errorText;
        switch (error)
        {
        case GL_INVALID_OPERATION:
            errorText = "INVALID_OPERATION"; 
            break;
        case GL_INVALID_ENUM:
            errorText = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            errorText = "INVALID_VALUE";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            errorText = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            errorText = "OUT_OF_MEMORY";
            break;
        default:
            errorText = "Invalid Enum Value, please debug.";
            break;
        }
        ERROR_RECOVERABLE(Stringf("OpenGL Error: %s\n File Name: %s at %i : \n", errorText, file, line));
    }
#endif
}

//-----------------------------------------------------------------------------------
void Renderer::EnableFaceCulling(bool enabled)
{
    enabled ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
}

//-----------------------------------------------------------------------------------
void Renderer::DrawPolygonOutline(const Vector2& center, float radius, int numSides, float radianOffset, const RGBA& color)
{
    Vertex_PCT* vertexes = new Vertex_PCT[numSides];
    const float radiansTotal = 2.0f * MathUtils::PI;
    const float radiansPerSide = radiansTotal / numSides;
    int index = 0;

    for (float radians = 0.0f; radians < radiansTotal; radians += radiansPerSide)
    {
        float adjustedRadians = radians + radianOffset;
        float x = center.x + (radius * cos(adjustedRadians));
        float y = center.y + (radius * sin(adjustedRadians));

        vertexes[index].color = color;
        vertexes[index].pos = Vector2(x, y);
    }
    DrawVertexArray(vertexes, numSides, DrawMode::LINE_LOOP);
}

//-----------------------------------------------------------------------------------
void Renderer::DrawPolygon(const Vector2& center, float radius, int numSides, float radianOffset, const RGBA& color)
{
    Vertex_PCT* vertexes = new Vertex_PCT[numSides];
    const float radiansTotal = 2.0f * MathUtils::PI;
    const float radiansPerSide = radiansTotal / numSides;
    int index = 0;

    for (float radians = 0.0f; radians < radiansTotal; radians += radiansPerSide)
    {
        float adjustedRadians = radians + radianOffset;
        float x = center.x + (radius * cos(adjustedRadians));
        float y = center.y + (radius * sin(adjustedRadians));

        vertexes[index].color = color;
        vertexes[index].pos = Vector2(x, y);
    }
    DrawVertexArray(vertexes, numSides, DrawMode::POLYGON);
}

//-----------------------------------------------------------------------------------
void Renderer::DrawAABB(const AABB2& bounds, const RGBA& color)
{
    const int NUM_VERTS = 4;
    SetColor(color);
    Vertex_PCT vertexes[NUM_VERTS];
    vertexes[0].color = color;
    vertexes[1].color = color;
    vertexes[2].color = color;
    vertexes[3].color = color;
    vertexes[0].pos = Vector3(bounds.mins.x, bounds.mins.y, 0.0f);
    vertexes[1].pos = Vector3(bounds.maxs.x, bounds.mins.y, 0.0f);
    vertexes[2].pos = Vector3(bounds.maxs.x, bounds.maxs.y, 0.0f);
    vertexes[3].pos = Vector3(bounds.mins.x, bounds.maxs.y, 0.0f);
    DrawVertexArray(vertexes, NUM_VERTS, DrawMode::QUADS);
}

//-----------------------------------------------------------------------------------
void Renderer::DrawAABB(const AABB3& bounds, const RGBA& color)
{
    const int NUM_VERTS = 24;
    SetColor(color);
    Vertex_PCT vertexes[NUM_VERTS];
    for (int i = 0; i < NUM_VERTS; i++)
    {
        vertexes[i].color = color;
    }
    //Bottom
    vertexes[0].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
    vertexes[1].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);
    vertexes[2].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);
    vertexes[3].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);

    //Top
    vertexes[4].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.maxs.z);
    vertexes[5].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.maxs.z);
    vertexes[6].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[7].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.maxs.z);

    //North
    vertexes[8].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);
    vertexes[9].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[10].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[11].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);

    //South
    vertexes[12].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
    vertexes[13].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);
    vertexes[14].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.maxs.z);
    vertexes[15].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.maxs.z);

    //East
    vertexes[16].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);
    vertexes[17].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);
    vertexes[18].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[19].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.maxs.z);

    //West
    vertexes[20].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
    vertexes[21].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.maxs.z);
    vertexes[22].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[23].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);

    DrawVertexArray(vertexes, NUM_VERTS, DrawMode::QUADS);
}

//-----------------------------------------------------------------------------------
void Renderer::DrawAABBBoundingBox(const AABB3& bounds, const RGBA& color)
{
    const int NUM_VERTS = 20;
    SetColor(color);
    Vertex_PCT vertexes[NUM_VERTS];
    for (int i = 0; i < NUM_VERTS; i++)
    {
        vertexes[i].color = color;
    }
    //Bottom
    vertexes[0].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
    vertexes[1].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);
    vertexes[2].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);
    vertexes[3].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);

    //West
    vertexes[4].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
    vertexes[5].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.maxs.z);
    vertexes[6].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[7].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);

    //North
    vertexes[8].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);
    vertexes[9].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[10].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[11].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);

    //East
    vertexes[12].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);
    vertexes[13].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);
    vertexes[14].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[15].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.maxs.z);

    //South
    vertexes[16].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);
    vertexes[17].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.maxs.z);
    vertexes[18].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.maxs.z);
    vertexes[19].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);

    DrawVertexArray(vertexes, NUM_VERTS, DrawMode::LINE_LOOP);
}

//-----------------------------------------------------------------------------------
void Renderer::DrawTexturedAABB3(const AABB3& bounds, const RGBA& color, const Vector2& texCoordMins, const Vector2& texCoordMaxs, Texture* texture)
{
    if (texture == nullptr)
    {
        texture = m_defaultTexture;
    }
    const int NUM_VERTS = 20;
    SetColor(color);
    Vertex_PCT vertexes[NUM_VERTS];
    for (int i = 0; i < NUM_VERTS; i++)
    {
        vertexes[i].color = color;
    }
    //Bottom
    vertexes[0].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
    vertexes[1].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);
    vertexes[2].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);
    vertexes[3].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);
    vertexes[0].texCoords = texCoordMins;
    vertexes[1].texCoords = Vector2(texCoordMaxs.x, texCoordMins.y);
    vertexes[2].texCoords = texCoordMaxs;
    vertexes[3].texCoords = Vector2(texCoordMins.x, texCoordMaxs.y);

    //West
    vertexes[4].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
    vertexes[5].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.maxs.z);
    vertexes[6].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[7].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);
    vertexes[4].texCoords = texCoordMins;
    vertexes[5].texCoords = Vector2(texCoordMaxs.x, texCoordMins.y);
    vertexes[6].texCoords = texCoordMaxs;
    vertexes[7].texCoords = Vector2(texCoordMins.x, texCoordMaxs.y);

    //North
    vertexes[8].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);
    vertexes[9].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[10].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[11].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);
    vertexes[8].texCoords = texCoordMins;
    vertexes[9].texCoords = Vector2(texCoordMaxs.x, texCoordMins.y);
    vertexes[10].texCoords = texCoordMaxs;
    vertexes[11].texCoords = Vector2(texCoordMins.x, texCoordMaxs.y);

    //East
    vertexes[12].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);
    vertexes[13].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);
    vertexes[14].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[15].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.maxs.z);
    vertexes[12].texCoords = texCoordMins;
    vertexes[13].texCoords = Vector2(texCoordMaxs.x, texCoordMins.y);
    vertexes[14].texCoords = texCoordMaxs;
    vertexes[15].texCoords = Vector2(texCoordMins.x, texCoordMaxs.y);

    //South
    vertexes[16].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);
    vertexes[17].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.maxs.z);
    vertexes[18].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.maxs.z);
    vertexes[19].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
    vertexes[16].texCoords = texCoordMins;
    vertexes[17].texCoords = Vector2(texCoordMaxs.x, texCoordMins.y);
    vertexes[18].texCoords = texCoordMaxs;
    vertexes[19].texCoords = Vector2(texCoordMins.x, texCoordMaxs.y);

    DrawVertexArray(vertexes, NUM_VERTS, DrawMode::QUADS);
}

//-----------------------------------------------------------------------------------
void Renderer::DrawTexturedAABB(const AABB2& bounds, const Vector2& texCoordMins, const Vector2& texCoordMaxs, Texture* texture, const RGBA& color)
{
    UNUSED(texture);
    Vertex_PCT vertexes[4];
    Vertex_PCT vertex;
    vertex.color = color;
    vertex.pos = Vector3(bounds.mins.x, bounds.mins.y, 0.0f);
    vertex.texCoords = texCoordMins;
    vertexes[0] = vertex;
    vertex.pos = Vector3(bounds.maxs.x, bounds.mins.y, 0.0f);
    vertex.texCoords = Vector2(texCoordMaxs.x, texCoordMins.y);
    vertexes[1] = vertex;
    vertex.pos = Vector3(bounds.maxs.x, bounds.maxs.y, 0.0f);
    vertex.texCoords = texCoordMaxs;
    vertexes[2] = vertex;
    vertex.pos = Vector3(bounds.mins.x, bounds.maxs.y, 0.0f);
    vertex.texCoords = Vector2(texCoordMins.x, texCoordMaxs.y);
    vertexes[3] = vertex;
    Renderer::instance->DrawVertexArray(vertexes, 4, DrawMode::QUADS);
}

//-----------------------------------------------------------------------------------
void Renderer::DrawTexturedFace(const Face& face, const Vector2& texCoordMins, const Vector2& texCoordMaxs, Texture* texture, const RGBA& color)
{
    UNUSED(texture);
    Vertex_PCT vertexes[4];
    Vertex_PCT vertex;
    vertex.color = color;
    vertex.pos = face.verts[0];
    vertex.texCoords = texCoordMins;
    vertexes[0] = vertex;
    vertex.pos = face.verts[1];
    vertex.texCoords = Vector2(texCoordMaxs.x, texCoordMins.y);
    vertexes[1] = vertex;
    vertex.pos = face.verts[2];
    vertex.texCoords = texCoordMaxs;
    vertexes[2] = vertex;
    vertex.pos = face.verts[3];
    vertex.texCoords = Vector2(texCoordMins.x, texCoordMaxs.y);
    vertexes[3] = vertex;
    Renderer::instance->DrawVertexArray(vertexes, 4, DrawMode::QUADS);
}

//-----------------------------------------------------------------------------------
void Renderer::BindFramebuffer(Framebuffer* fbo)
{
    if (m_fbo == fbo)
    {
        return;
    }

    m_fbo = fbo;
    if (fbo == nullptr)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, NULL);
        #pragma TODO("Make aspect not hard-coded!!!")
        glViewport(0, 0, 1600, 900);

    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo->m_fboHandle);
        glViewport(0, 0, fbo->m_pixelWidth, fbo->m_pixelHeight);

        GLenum renderTargets[32];
        memset(renderTargets, 0, sizeof(renderTargets));
        for (uint32_t i = 0; i < fbo->m_colorCount; ++i)
        {
            renderTargets[i] = GL_COLOR_ATTACHMENT0 + i;
        }

        glDrawBuffers(fbo->m_colorCount, //How many
            renderTargets); //What do they render to?

    }
}

//-----------------------------------------------------------------------------------
//Is really only used for debug purposes
void Renderer::FrameBufferCopyToBack(Framebuffer* fbo, int colorTargetNumber /*= NULL*/)
{
    if (fbo == nullptr)
    {
        return;
    }

    GLuint fboHandle = fbo->m_fboHandle;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboHandle);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, NULL);

    uint32_t readWidth = fbo->m_pixelWidth;
    uint32_t readHeight = fbo->m_pixelHeight;

#pragma TODO("Make aspect not hard-coded!!!")
    uint32_t drawWidth = 1600;
    uint32_t drawHeight = 900;

    glBlitFramebuffer(0, 0, //Lower left corner pixel
        readWidth, readHeight, //Top right corner pixel
        0, 0, //lower left pixel of the read buffer
        drawWidth, drawHeight, //top right pixel of read buffer
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST);

    GL_CHECK_ERROR();
}