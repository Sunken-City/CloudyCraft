#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Vertex.hpp"
#include "Engine/Math/Vector3Int.hpp"
#include <vector>

DebugRenderer* DebugRenderer::instance = nullptr;
std::set<DebugRenderer::Command*> DebugRenderer::s_renderCommands;

//-----------------------------------------------------------------------------------
void DebugRenderer::Update(float deltaSeconds)
{
    for (auto commandIter = s_renderCommands.begin(); commandIter != s_renderCommands.end(); ++commandIter)
    {
        DebugRenderer::Command* command = *commandIter;
        command->m_duration -= deltaSeconds;
        if (command->HasExpired())
        {
            commandIter = s_renderCommands.erase(commandIter);
            if (commandIter == s_renderCommands.end())
            {
                break;
            }
        }
    }
}

//-----------------------------------------------------------------------------------
void DebugRenderer::Render() const
{
    for (Command* command : s_renderCommands)
    {
        command->Render();
    }
}

//-----------------------------------------------------------------------------------
void DebugRenderer::DrawDebugPoint(const Vector3& position, const RGBA& color, float duration, DepthTestingMode mode)
{
    DebugRenderer::Command* command = new DebugRenderer::PointCommand(position, color, duration, mode);
    s_renderCommands.emplace(command);
}

//-----------------------------------------------------------------------------------
void DebugRenderer::DrawDebugLine(const Vector3& start, const Vector3& end, const RGBA& color, float duration, DepthTestingMode mode)
{
    DebugRenderer::Command* command = new DebugRenderer::LineCommand(start, end, color, duration, mode);
    s_renderCommands.emplace(command);
}

//-----------------------------------------------------------------------------------
void DebugRenderer::DrawDebugArrow(const Vector3& start, const Vector3& forward, const RGBA& color, float duration, DepthTestingMode mode)
{
    DebugRenderer::Command* command = new DebugRenderer::ArrowCommand(start, forward, color, duration, mode);
    s_renderCommands.emplace(command);
}

//-----------------------------------------------------------------------------------
void DebugRenderer::DrawDebugAABB3(const AABB3& bounds, const RGBA& strokeColor, const RGBA& fillColor, float duration, DepthTestingMode mode)
{
    DebugRenderer::Command* command = new DebugRenderer::AABB3Command(bounds, strokeColor, fillColor, duration, mode);
    s_renderCommands.emplace(command);
}

//-----------------------------------------------------------------------------------
void DebugRenderer::DrawDebugSphere(const Vector3& position, float radius, const RGBA& color, float duration, DepthTestingMode mode)
{
    DebugRenderer::Command* command = new DebugRenderer::SphereCommand(position, radius, color, duration, mode);
    s_renderCommands.emplace(command);
}

//-----------------------------------------------------------------------------------
DebugRenderer::Command::Command()
{

}

//-----------------------------------------------------------------------------------
void DebugRenderer::Command::Render() const
{

}

//-----------------------------------------------------------------------------------
DebugRenderer::PointCommand::PointCommand(const Vector3& position, const RGBA& color, float duration, DebugRenderer::DepthTestingMode mode)
    : m_position(position)
{
    m_color = color;
    m_duration = duration;
    m_mode = mode;
}

//-----------------------------------------------------------------------------------
void DebugRenderer::PointCommand::Render() const
{
    bool depthTestOn = m_mode == DepthTestingMode::OFF ? false : true;
    Renderer::instance->EnableDepthTest(depthTestOn);
    Renderer::instance->DrawLine(m_position - Vector3::UNIT_X, m_position + Vector3::UNIT_X, m_color, 4.0f);
    Renderer::instance->DrawLine(m_position - Vector3::UNIT_Y, m_position + Vector3::UNIT_Y, m_color, 4.0f);
    Renderer::instance->DrawLine(m_position - Vector3::UNIT_Z, m_position + Vector3::UNIT_Z, m_color, 4.0f);
    if (m_mode == DepthTestingMode::XRAY)
    {
        Renderer::instance->EnableDepthTest(false);
        Renderer::instance->DrawLine(m_position - Vector3::UNIT_X, m_position + Vector3::UNIT_X, m_color, 1.0f);
        Renderer::instance->DrawLine(m_position - Vector3::UNIT_Y, m_position + Vector3::UNIT_Y, m_color, 1.0f);
        Renderer::instance->DrawLine(m_position - Vector3::UNIT_Z, m_position + Vector3::UNIT_Z, m_color, 1.0f);
    }
}

//-----------------------------------------------------------------------------------
DebugRenderer::LineCommand::LineCommand(const Vector3& start, const Vector3& end, const RGBA& color, float duration, DebugRenderer::DepthTestingMode mode)
    : m_start(start)
    , m_end(end)
{
    m_color = color;
    m_duration = duration;
    m_mode = mode;
}

//-----------------------------------------------------------------------------------
void DebugRenderer::LineCommand::Render() const
{
    bool depthTestOn = m_mode == DepthTestingMode::OFF ? false : true;
    Renderer::instance->EnableDepthTest(depthTestOn);
    Renderer::instance->DrawLine(m_start, m_end, m_color, 6.0f);
    if (m_mode == DepthTestingMode::XRAY)
    {
        Renderer::instance->EnableDepthTest(false);
        Renderer::instance->DrawLine(m_start, m_end, m_color, 2.0f);
    }
}

//-----------------------------------------------------------------------------------
DebugRenderer::ArrowCommand::ArrowCommand(const Vector3& start, const Vector3& end, const RGBA& color, float duration, DebugRenderer::DepthTestingMode mode)
    : m_start(start)
    , m_end(end)
{
    m_color = color;
    m_duration = duration;
    m_mode = mode;
}

//-----------------------------------------------------------------------------------
void DebugRenderer::ArrowCommand::Render() const
{
    bool depthTestOn = m_mode == DepthTestingMode::OFF ? false : true;
    Renderer::instance->EnableDepthTest(depthTestOn);
    Renderer::instance->DrawLine(m_start, m_end, m_color, 6.0f);
    Renderer::instance->DrawPoint(m_end - Vector3::GetNormalized(m_end), m_color, 20.0f);
    if (m_mode == DepthTestingMode::XRAY)
    {
        Renderer::instance->EnableDepthTest(false);
        Renderer::instance->DrawLine(m_start, m_end, m_color, 2.0f);
        Renderer::instance->DrawPoint(m_end - Vector3::GetNormalized(m_end), m_color, 10.0f);
    }
}

//-----------------------------------------------------------------------------------
DebugRenderer::SphereCommand::SphereCommand(const Vector3& position, float radius, const RGBA& color, float duration, DepthTestingMode mode)
    : m_position(position)
    , m_radius(radius)
{
    m_color = color;
    m_duration = duration;
    m_mode = mode;
    //Generate points to use for the sphere algorithm
    //We need to start off with an octahedron to keep the faces even.
    std::vector<Vertex_PCT> verts;
    std::vector<Vector3Int> indices;
    Vector3 initialPoints[6] = { { 0, 0, radius }, { 0, 0, -radius }, { -radius, -radius, 0 }, { radius, -radius, 0 }, { radius, radius, 0 }, { -radius,  radius, 0 } };
    Vertex_PCT vert;
    vert.color = color;

    for (int i = 0; i < 6; i++)
    {
        vert.pos = Vector3::GetNormalized(initialPoints[i]) * radius;
        verts.push_back(vert);
    }

    indices.push_back(Vector3Int(0, 3, 4));
    indices.push_back(Vector3Int(0, 4, 5));
    indices.push_back(Vector3Int(0, 5, 2));
    indices.push_back(Vector3Int(0, 2, 3));
    indices.push_back(Vector3Int(1, 4, 3));
    indices.push_back(Vector3Int(1, 5, 4));
    indices.push_back(Vector3Int(1, 2, 5));
    indices.push_back(Vector3Int(1, 3, 2));

    for (int i = 0; i < 3; i++)
    {
        int n = indices.size();
        for (int j = 0; j < n; j++)
        {
            // Calculate the midpoints 
            Vector3 point1 = Vector3::GetMidpoint(verts.at(indices.at(j).x).pos, verts.at(indices.at(j).y).pos);
            Vector3 point2 = Vector3::GetMidpoint(verts.at(indices.at(j).y).pos, verts.at(indices.at(j).z).pos);
            Vector3 point3 = Vector3::GetMidpoint(verts.at(indices.at(j).z).pos, verts.at(indices.at(j).x).pos);

            //Move the points to the outside of our sphere.
            point1.Normalize();
            point2.Normalize();
            point3.Normalize();

            //Add these vertices to the list of verts, and store their array location.
            //Naiive way to check for dupes is here. I'll find a better solution later.
            int point1Location = -1;
            int point2Location = -1;
            int point3Location = -1;
            for (unsigned int q = 0; q < verts.size(); q++)
            {
                Vector3 compare = verts.at(q).pos;
                if (compare == point1)
                    point1Location = q;
                if (compare == point2)
                    point2Location = q;
                if (compare == point3)
                    point3Location = q;
            }
            if (point1Location == -1)
            {
                point1Location = verts.size(); 
                vert.pos = point1 * radius;
                verts.push_back(vert);
            }

            if (point2Location == -1)
            {
                point2Location = verts.size();
                vert.pos = point2 * radius;
                verts.push_back(vert);
            }

            if (point3Location == -1)
            {
                point3Location = verts.size();
                vert.pos = point3 * radius;
                verts.push_back(vert);
            }

            //Create 3 new faces (the outer triangles, the pieces of the triforce)
            indices.push_back(Vector3Int(indices.at(j).x, point1Location, point3Location));
            indices.push_back(Vector3Int(point1Location, indices.at(j).y, point2Location));
            indices.push_back(Vector3Int(point3Location, point2Location, indices.at(j).z));

            //Replace the original face with the inner, upside-down triangle (not the triforce)
            indices.at(j).x = point1Location;
            indices.at(j).y = point2Location;
            indices.at(j).z = point3Location;
        }
    }
    //Send sphere to the graphics card
    m_vboId = Renderer::instance->GenerateBufferID();
    m_numVerts = verts.size();
    Renderer::instance->BindAndBufferVBOData(m_vboId, verts.data(), m_numVerts);
}

//-----------------------------------------------------------------------------------
DebugRenderer::SphereCommand::~SphereCommand()
{
    Renderer::instance->DeleteBuffers(m_vboId);
}

//-----------------------------------------------------------------------------------
void DebugRenderer::SphereCommand::Render() const
{
    Renderer::instance->PushMatrix();
    {
        Renderer::instance->Translate(m_position);

        bool depthTestOn = m_mode == DepthTestingMode::OFF ? false : true;
        Renderer::instance->EnableDepthTest(depthTestOn);
        Renderer::instance->SetPointSize(5.0f);
        Renderer::instance->DrawVBO_PCT(m_vboId, m_numVerts, Renderer::DrawMode::POINTS);
        if (m_mode == DepthTestingMode::XRAY)
        {
            Renderer::instance->EnableDepthTest(false);
            Renderer::instance->SetPointSize(2.0f);
            Renderer::instance->DrawVBO_PCT(m_vboId, m_numVerts, Renderer::DrawMode::POINTS);
        }
    }
    Renderer::instance->PopMatrix();
}

//-----------------------------------------------------------------------------------
DebugRenderer::AABB3Command::AABB3Command(const AABB3& bounds, const RGBA& strokeColor, const RGBA& fillColor, float duration, DepthTestingMode mode)
    : m_bounds(bounds)
    , m_fillColor(fillColor)
{
    m_color = strokeColor;
    m_duration = duration;
    m_mode = mode;
}

//-----------------------------------------------------------------------------------
void DebugRenderer::AABB3Command::Render() const
{
    bool depthTestOn = m_mode == DepthTestingMode::OFF ? false : true;
    Renderer::instance->EnableDepthTest(depthTestOn);
    Renderer::instance->SetLineWidth(7.0f);
    Renderer::instance->DrawTexturedAABB3(m_bounds, m_fillColor);
    Renderer::instance->DrawAABBBoundingBox(m_bounds, m_color);
    if (m_mode == DepthTestingMode::XRAY)
    {
        Renderer::instance->EnableDepthTest(false);
        Renderer::instance->SetLineWidth(1.0f);
        RGBA fadedColor = m_fillColor;
        fadedColor.alpha = 0x80;
        Renderer::instance->DrawTexturedAABB3(m_bounds, fadedColor);
        Renderer::instance->DrawAABBBoundingBox(m_bounds, m_color);
    }
}
