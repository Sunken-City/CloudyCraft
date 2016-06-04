#include "Engine/Core/Memory/MemoryOutputWindow.hpp"
#include "Engine/Core/Memory/MemoryTracking.hpp"
#include "Engine/Core/BuildConfig.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Input/XMLUtils.hpp"
#include "Engine/TextRendering/TextBox.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Input/Console.hpp"

MemoryOutputWindow* MemoryOutputWindow::instance = nullptr;

//-----------------------------------------------------------------------------------
MemoryOutputWindow::MemoryOutputWindow()
    : m_outputWindow(nullptr)
    , m_isHidden(false)
{
}

//-----------------------------------------------------------------------------------
MemoryOutputWindow::~MemoryOutputWindow()
{
    delete m_outputWindow;
}

#if defined(TRACK_MEMORY)
//-----------------------------------------------------------------------------------
void MemoryOutputWindow::Update(float deltaSeconds)
{
    if (m_isHidden)
    {
        return;
    }
    if (!m_outputWindow)
    {
        m_outputWindow = new TextBox(Vector3(1300.0f, 500.0f, 0.0f), Vector3::UP, Vector3::RIGHT, 300, 300, 1.0f, BitmapFont::CreateOrGetFontFromGlyphSheet("Runescape"), TextBox::LEFT_ALIGNED);
    }
    std::string xmlData = Stringf("<Text value = \"Allocations: [[%u]]\
                                                   Bytes: [[%u]]\
                                                   Highwater: [[%u]]\" color = \"FFFFFF\">\
                                        <Effect color1 = \"FF0000\" color2 = \"FFFF00\" />                    \
                                        <Effect color1 = \"FF0000\" color2 = \"FFFF00\" />                    \
                                        <Effect color1 = \"FF0000\" color2 = \"FFFF00\" />                    \
                                       </Text>"
        , g_memoryAnalytics.m_numberOfAllocations
        , g_memoryAnalytics.m_numberOfBytes
        , g_memoryAnalytics.m_highwaterInBytes);
    m_outputWindow->SetFromXMLNode(XMLUtils::ParseXMLFromString(xmlData));
    m_outputWindow->Update(deltaSeconds);
}

//-----------------------------------------------------------------------------------
void MemoryOutputWindow::Render() const
{
    if (m_isHidden)
    {
        return;
    }
    Renderer::instance->BeginOrtho(Vector2(0.0f, 0.0f), Vector2(1600, 900));
    m_outputWindow->Render();
    Renderer::instance->EndOrtho();
}

CONSOLE_COMMAND(memoryDebug)
{
    MemoryOutputWindow::instance->m_isHidden = !MemoryOutputWindow::instance->m_isHidden;
    Console::instance->PrintLine("Memory debugging toggled", RGBA::VAPORWAVE);
}

#else

void MemoryOutputWindow::Update(float deltaSeconds) {};
void MemoryOutputWindow::Render() const {};

#endif