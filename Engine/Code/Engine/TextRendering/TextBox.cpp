#include "Engine/TextRendering/TextBox.hpp"
#include "Engine/Input/XMLUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"
#include <deque>

//-----------------------------------------------------------------------------------------------
TextBox::TextBox(const Vector3& bottomLeft, const Vector3& upVector, const Vector3& rightVector, float width, float height, float scale, class BitmapFont* baseFont, TextAlignment textAlignment /*= TextAlignment::LEFT_ALIGNED*/) 
    : m_bottomLeft(bottomLeft)
    , m_upVector(upVector)
    , m_rightVector(rightVector)
    , m_baseFont(baseFont)
    , m_width(width)
    , m_height(height)
    , m_scale(scale)
    , m_totalTimeSinceReset(0.f)
    , m_renderBox(false)
    , m_alignment(textAlignment)
    , m_borderRenderer(nullptr)
    , m_textRenderers()
    //, m_textShader(new ShaderProgram("Data/Shaders/funkyFont.vert", "Data/Shaders/funkyFont.frag"))
    //, m_backgroundRenderer(nullptr)
{
}

//-----------------------------------------------------------------------------------
TextBox::~TextBox()
{
    if (m_borderRenderer)
    {
        delete m_borderRenderer->m_mesh;
        delete m_borderRenderer;
    }
    DeleteTextRenderers();
}

//-----------------------------------------------------------------------------------------------
void TextBox::EvaluateLine(std::deque<StringEffectFragment>& currLine, std::deque<StringEffectFragment>& fragmentQueue)
{
    std::string fullLineString = "";
    std::deque<StringEffectFragment> workingLine;
    if (currLine.size() == 1)
    {
        return;
    }
    while(!currLine.empty())
    {
        workingLine.push_back(currLine.front());
        currLine.pop_front();
        std::string currString = workingLine.back().m_value;
        fullLineString += currString;
        float width = m_baseFont->CalcTextWidth(fullLineString, m_scale);
        if (width > m_width)
        {
            for (size_t i = 0; i < currString.size(); i++)
            {
                fullLineString.pop_back();
            }
            if (currString.find(" ") == std::string::npos)
            {
                currLine.push_front(workingLine.back());
                TrimBeginning(currLine[0].m_value);
                workingLine.pop_back();
                currLine.push_front(StringEffectFragment("\n"));
                size_t workingLineSize = workingLine.size();
                for (size_t i = 0; i < workingLineSize; i++)
                {
                    if (i == 0)
                    {
                        TrimEnd(workingLine.back().m_value);
                    }
                    currLine.push_front(workingLine.back());
                    workingLine.pop_back();
                }
            }
            else
            {
                StringEffectFragment badFrag = workingLine.back();
                workingLine.pop_back();
                std::string unprocessedText = badFrag.m_value;
                std::string processedText = "";
                for (;;)
                {
                    if (unprocessedText.empty())
                    {
                        break;
                    }
                    char testChar = unprocessedText[0];
                    processedText.push_back(testChar);
                    unprocessedText = unprocessedText.substr(1);
                    if (testChar == ' ')
                    {
                        std::string testString = fullLineString + processedText;
                        if (m_baseFont->CalcTextWidth(testString, m_scale) > m_width)
                        {
                            for (char rewindChar = processedText.back(); !processedText.empty() && rewindChar != ' '; rewindChar = processedText.back())
                            {
                                processedText.pop_back();
                                unprocessedText = rewindChar + unprocessedText;
                            }
                            TrimBeginning(unprocessedText);
                            TrimEnd(processedText);
                            if (unprocessedText != "")
                            {
                                StringEffectFragment unprocessedFrag(unprocessedText);
                                unprocessedFrag.m_effect = badFrag.m_effect;
                                currLine.push_front(unprocessedFrag);
                            }
                            currLine.push_front(StringEffectFragment("\n"));
                            if (processedText != "")
                            {
                                StringEffectFragment processedFrag(processedText);
                                processedFrag.m_effect = badFrag.m_effect;
                                currLine.push_front(processedFrag);
                            }
                            size_t workingLineSize = workingLine.size();
                            for (size_t i = 0; i < workingLineSize; i++)
                            {
                                currLine.push_front(workingLine.back());
                                workingLine.pop_back();
                            }
                            break;
                        }
                    }
                }
            }
            break;
        }
    }
    if (currLine.empty())
    {
        std::swap(currLine, workingLine);
    }
    else
    {
        for (size_t i = 0; i < currLine.size(); i++)
        {
            fragmentQueue.push_front(currLine.back());
            currLine.pop_back();
        }
    }
}


//-----------------------------------------------------------------------------------------------
static void SetEffectProperties(Material* mat, const StringEffectFragment& frag)
{
    mat->SetFloatUniform("gWave", frag.m_effect.wave);
    mat->SetIntUniform("gShake", (int)frag.m_effect.shake);
    mat->SetFloatUniform("gDilate", frag.m_effect.dilate);
    mat->SetIntUniform("gPop", (int)frag.m_effect.pop);
    mat->SetVec4Uniform("gColor1", frag.m_effect.color1.ToVec4());
    mat->SetVec4Uniform("gColor2", frag.m_effect.color2.ToVec4());
    mat->SetIntUniform("gRainbow", (int)frag.m_effect.rainbow);
}

//-----------------------------------------------------------------------------------------------
void TextBox::ConstructMeshes()
{
    //Delete if already initialized
    DeleteTextRenderers();
    float totalStringWidth = 0.f;
    std::vector<float>lineWidths;
    float currLineWidth = 0.f;
    for (StringEffectFragment& frag : m_fragments)
    {
        if (frag.m_value == "\n")
        {
            lineWidths.push_back(currLineWidth);
            currLineWidth = 0.f;
            continue;
        }

        totalStringWidth += m_baseFont->CalcTextWidth(frag.m_value, m_scale);
        currLineWidth += m_baseFont->CalcTextWidth(frag.m_value, m_scale);
    }
    lineWidths.push_back(currLineWidth);
    float totalWidthUpToNow = 0.f;
    int lineNum = 0;
    float alignment = 0.0f; //Left Aligned
    alignment = m_alignment == CENTER_ALIGNED ? 0.5f : 0.0f;
    alignment = m_alignment == RIGHT_ALIGNED ? 1.0f : 0.0f;
    for (StringEffectFragment& frag : m_fragments)
    {
        MeshBuilder mb;
        mb.AddStringEffectFragment(frag.m_value, m_baseFont, m_scale, totalStringWidth, totalWidthUpToNow, Vector3::ZERO
                                , m_upVector, m_rightVector, m_width, m_height, lineNum, lineWidths[lineNum], alignment);
        Mesh* mesh = new Mesh();
        mb.CopyToMesh(mesh, &Vertex_TextPCT::Copy, sizeof(Vertex_TextPCT), &Vertex_TextPCT::BindMeshToVAO);
        Material* mat = new Material(new ShaderProgram("Data/Shaders/funkyFont.vert", "Data/Shaders/funkyFont.frag"), RenderState());
        SetEffectProperties(mat, frag);
        mat->SetVec3Uniform("gUpVector", m_upVector);
        mat->SetVec3Uniform("gRightVector", m_rightVector);
        mat->SetDiffuseTexture(m_baseFont->GetTexture());
        MeshRenderer* meshRenderer = new MeshRenderer(mesh, mat);
        meshRenderer->SetPosition(m_bottomLeft);
        m_textRenderers.push_back(meshRenderer);
        totalWidthUpToNow += m_baseFont->CalcTextWidth(frag.m_value, m_scale);
        if (frag.m_value == "\n")
        {
            lineNum++;
            totalWidthUpToNow = 0.f;
        }
    }
}

//-----------------------------------------------------------------------------------
void TextBox::InitializeBorder()
{
    //Add mesh renderers for the text box
    MeshBuilder builder;
    Vector3 br = (m_rightVector * m_width);
    Vector3 tl = (m_upVector * m_height);
    builder.AddLine(Vector3::ZERO, br);
    builder.AddLine(Vector3::ZERO, tl);
    builder.AddLine(tl, br + (m_upVector * m_height));
    builder.AddLine(br, br + (m_upVector * m_height));
    m_borderRenderer = new MeshRenderer(new Mesh(), Renderer::instance->m_defaultMaterial);
    builder.CopyToMesh(m_borderRenderer->m_mesh, &Vertex_PCT::Copy, sizeof(Vertex_PCT), &Vertex_PCT::BindMeshToVAO);
}

//-----------------------------------------------------------------------------------
void TextBox::DeleteTextRenderers()
{
    for (MeshRenderer* renderer : m_textRenderers)
    {
        delete renderer->m_mesh;
        delete renderer->m_material->m_shaderProgram;
        delete renderer->m_material;
        delete renderer;
        renderer = nullptr;
    }
    m_textRenderers.clear();
}

//-----------------------------------------------------------------------------------------------
void TextBox::ResetAnimation()
{
    m_totalTimeSinceReset = 0.f;
}

//-----------------------------------------------------------------------------------------------
void TextBox::SetFromXMLNode(const struct XMLNode& node)
{
    if (m_fragments.size() > 0)
    {
        m_fragments.clear();
    }
    m_fragments = StringEffectFragment::GetStringFragmentsFromXML(node);

    std::deque<StringEffectFragment> fragmentQueue;

    for (StringEffectFragment& frag : m_fragments)
    {
        fragmentQueue.push_back(frag);
    }

    m_fragments.clear();
    
    while (!fragmentQueue.empty())
    {
        std::deque<StringEffectFragment> currLine;
        for (StringEffectFragment frag: fragmentQueue)
        {
            currLine.push_back(frag);
            if (frag.m_value == "\n")
            {
                break;
            }
        }
        for (size_t i = 0; i < currLine.size(); i++)
        {
            fragmentQueue.pop_front();
        }
        EvaluateLine(currLine, fragmentQueue);
        while (!currLine.empty())
        {
            m_fragments.push_back(currLine.front());
            currLine.pop_front();
        }
    }

    for (auto iter = m_fragments.begin(); iter != m_fragments.end();)
    {
        if (iter->m_value == "")
        {
            m_fragments.erase(iter++);
        }
        else
        {
            iter++;
        }
    }

    ConstructMeshes();
}

//-----------------------------------------------------------------------------------------------
void TextBox::Update(float deltaSeconds)
{
    m_totalTimeSinceReset += deltaSeconds;
    if (m_renderBox)
    {
        if (m_borderRenderer)
        {
            m_borderRenderer->SetPosition(m_bottomLeft);
        }
        else
        {
            InitializeBorder();
        }
    }
    for (MeshRenderer* mr : m_textRenderers)
    {
        mr->SetPosition(m_bottomLeft);
        mr->m_material->SetFloatUniform("gTime", m_totalTimeSinceReset);
    }
}

//-----------------------------------------------------------------------------------------------
void TextBox::Render() const
{
    if (m_renderBox && m_borderRenderer)
    {
        m_borderRenderer->Render();
    }
    for (const MeshRenderer* mr : m_textRenderers)
    {
        if (mr)
            mr->Render();
    }
}