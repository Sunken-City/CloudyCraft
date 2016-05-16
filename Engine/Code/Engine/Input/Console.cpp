#include "Engine/Input/Console.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include <cmath>

Console* Console::instance = nullptr;
std::map<std::string, ConsoleCommandFunctionPointer> *g_consoleCommands = nullptr;

const float Console::CHARACTER_HEIGHT = 20.0f;
const float Console::CHARACTER_WIDTH = 15.0f;
const float Console::CURSOR_BLINK_RATE_SECONDS = 0.5f;
const char Console::CURSOR_CHARACTER = 0x7C; //| , 0xDB []

//-----------------------------------------------------------------------------------
Console::Console()
	: m_currentLine(new char[MAX_LINE_LENGTH]())
	, m_cursorPointer(m_currentLine)
	, m_isActive(false)
	, m_isCursorShowing(false)
	, m_characterAtCursor(CURSOR_CHARACTER)
	, m_timeSinceCursorBlink(0.0f)
	, m_font(BitmapFont::CreateOrGetFontFromGlyphSheet("Arial"))
{
}

//-----------------------------------------------------------------------------------
Console::~Console()
{
}

//-----------------------------------------------------------------------------------
void Console::Update(float deltaSeconds)
{
	if (m_isActive)
	{
		m_timeSinceCursorBlink += deltaSeconds;

		if (*m_cursorPointer != CURSOR_CHARACTER)
		{
			m_characterAtCursor = *m_cursorPointer;
		}

		char currentChar = InputSystem::instance->GetLastPressedChar();
		ParseKey(currentChar);

		if (m_timeSinceCursorBlink >= CURSOR_BLINK_RATE_SECONDS)
		{
			m_timeSinceCursorBlink = 0.0f;
			m_isCursorShowing = !m_isCursorShowing;
			*m_cursorPointer = m_isCursorShowing ? CURSOR_CHARACTER : m_characterAtCursor;
		}
	}
}

//-----------------------------------------------------------------------------------
void Console::ParseKey(char currentChar)
{
	if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::ESC))
	{
		*m_cursorPointer = m_characterAtCursor;
		if (IsEmpty())
		{
			DeactivateConsole();
			return;
		}
		else
		{
			m_cursorPointer = m_currentLine;
			memset(m_currentLine, 0x00, MAX_LINE_LENGTH);
		}
	}
	if (currentChar > 0x1F && m_cursorPointer != (m_currentLine + MAX_LINE_LENGTH))
	{
		*m_cursorPointer = currentChar;
		m_cursorPointer++;
	}
	else if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::BACKSPACE) && m_cursorPointer != m_currentLine)
	{
		*m_cursorPointer = m_characterAtCursor;
		m_cursorPointer--;
		*m_cursorPointer = '\0';
	}
	else if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::ENTER))
	{
		*m_cursorPointer = m_characterAtCursor;
		if (IsEmpty())
		{
			DeactivateConsole();
			return;
		}
		std::string currentLine = std::string(m_currentLine);
		m_consoleHistory.push_back(ColoredText(currentLine, RGBA::GRAY));
		if (!RunCommand(currentLine))
		{
			m_consoleHistory.push_back(ColoredText("Invalid Command.", RGBA::MAROON));
		}
		m_cursorPointer = m_currentLine;
		memset(m_currentLine, 0x00, MAX_LINE_LENGTH);
	}
	else if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::LEFT) && m_cursorPointer != m_currentLine)
	{
		*m_cursorPointer = m_characterAtCursor;
		m_cursorPointer--;
	}
	else if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::RIGHT) && m_cursorPointer != (m_currentLine + MAX_LINE_LENGTH))
	{
		*m_cursorPointer = m_characterAtCursor;
		m_cursorPointer++;
	}
	else if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::HOME))
	{
		*m_cursorPointer = m_characterAtCursor;
		m_cursorPointer = m_currentLine;
	}
	else if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::END))
	{
		*m_cursorPointer = m_characterAtCursor;
		for (int i = 0; i < MAX_LINE_LENGTH; i++)
		{
			char* currentIndex = m_cursorPointer + i;
			if (*currentIndex == '\0')
			{
				m_cursorPointer = currentIndex;
				break;
			}
		}
	}
	else if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::DEL))
	{
		const int maxLengthInFrontOfCursor = MAX_LINE_LENGTH - (m_cursorPointer - m_currentLine);
		for (int i = 0; i < maxLengthInFrontOfCursor; i++)
		{
			char* currentIndex = m_cursorPointer + i;
			char* nextIndex = currentIndex + 1;
			*currentIndex = *nextIndex;
			if (*nextIndex == '\0')
			{
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------------
void Console::Render() const
{
	if (m_isActive)
	{
		Renderer::instance->BeginOrtho(Vector2(0.0f, 0.0f), Vector2(1600, 900));
		Renderer::instance->EnableDepthTest(false);
		Renderer::instance->DrawAABB(AABB2(Vector2(0, 0), Vector2(1600, 900)), RGBA(0x00000088));

		Vector2 currentBaseline = Vector2::ONE * 10.0f;
		Renderer::instance->DrawText2D(currentBaseline, std::string(m_currentLine), 1.0f, RGBA::WHITE, true, m_font);
		unsigned int index = m_consoleHistory.size() - 1;
		unsigned int numberOfLinesPrinted = 0;
		for (auto reverseIterator = m_consoleHistory.rbegin(); reverseIterator != m_consoleHistory.rend(); ++reverseIterator, --index)
		{
			currentBaseline += Vector2(0.0f, (float)m_font->m_maxHeight);
			Renderer::instance->DrawText2D(currentBaseline, m_consoleHistory[index].text, 1.0f, m_consoleHistory[index].color, true, m_font);
			numberOfLinesPrinted++;
			if (numberOfLinesPrinted > MAX_CONSOLE_LINES)
			{
				return;
			}
		}
		Renderer::instance->EndOrtho();
	}
}

//-----------------------------------------------------------------------------------
void Console::ToggleConsole()
{
	m_isActive ? DeactivateConsole() : ActivateConsole();
}

//-----------------------------------------------------------------------------------
void Console::ActivateConsole()
{
	m_isActive = true;
}

//-----------------------------------------------------------------------------------
void Console::DeactivateConsole()
{
	m_isActive = false;
}

//-----------------------------------------------------------------------------------
void Console::ClearConsoleHistory()
{
	m_consoleHistory.clear();
}

//-----------------------------------------------------------------------------------
void Console::RegisterCommand(std::string commandName, ConsoleCommandFunctionPointer consoleFunction)
{
	g_consoleCommands->emplace(commandName, consoleFunction);
}

//-----------------------------------------------------------------------------------
void Console::PrintLine(std::string consoleLine, RGBA color)
{
	m_consoleHistory.push_back(ColoredText(consoleLine, color));
}

//-----------------------------------------------------------------------------------
//Returns true if command was found and run, false if invalid.
bool Console::RunCommand(std::string commandLine)
{
	Command command(commandLine);

	auto iterator = g_consoleCommands->find(command.GetCommandName());
	if (iterator != g_consoleCommands->end())
	{
		ConsoleCommandFunctionPointer outCommand = (*iterator).second;
		outCommand(command);
		return true;
	};
	return false;
}

//-----------------------------------------------------------------------------------
Command::Command(std::string fullCommandStr)
	: m_fullCommandStr(fullCommandStr)
{
	char* charLine = (char*)fullCommandStr.c_str();
	char* token = nullptr;
	char* context = nullptr;
	char delimiters[] = " ";

	token = strtok_s(charLine, delimiters, &context);
	//First "arg" is the name
	if (token == nullptr)
	{
		m_commandName = std::string("INVALID_COMMAND");
	}
	else
	{
		m_commandName = std::string(token);
		token = strtok_s(NULL, delimiters, &context);

		while (token != nullptr)
		{
			m_argsList.push_back(std::string(token));
			token = strtok_s(NULL, delimiters, &context);
		}
	}
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(help)
{
	if (args.HasArgs(0))
	{
		Console::instance->PrintLine("Console Controls:", RGBA::WHITE);
		Console::instance->PrintLine("Enter ~ Run command / Close console (if line empty)", RGBA::GRAY);
		Console::instance->PrintLine("All registered commands:", RGBA::WHITE);
		float i = 0.0f;
		for (auto commandNameFunctionPair : *g_consoleCommands)
		{
			float frequency = 3.14f * 2.0f / (float)g_consoleCommands->size();
			float center = 0.5f;
			float width = 0.49f;
			float red = sin(frequency * i + 2.0f) * width + center;
			float green = sin(frequency * i + 0.0f) * width + center;
			float blue = sin(frequency * i + 4.0f) * width + center;
			Console::instance->PrintLine(commandNameFunctionPair.first, RGBA(red, green, blue));
			i += 1.0f;
		}
		return;
	}
	if (!args.HasArgs(1))
	{
		Console::instance->PrintLine("help <string>", RGBA::GRAY);
		return;
	}
	std::string arg0 = args.GetStringArgument(0);
	if (arg0 == "help")
	{
		Console::instance->PrintLine("help: A command (that you just used) to find more info on other commands! Success! :D", RGBA::GRAY);
	}
	else if (arg0 == "clear")
	{
		Console::instance->PrintLine("clear: Clears the command history for the console", RGBA::GRAY);
	}
	else if (arg0 == "quit")
	{
		Console::instance->PrintLine("quit: Quits the application after saving any data.", RGBA::GRAY);
	}
	else if (arg0 == "motd")
	{
		Console::instance->PrintLine("motd: Displays the Message of the Day", RGBA::GRAY);
	}
	else if (arg0 == "runfor")
	{
		Console::instance->PrintLine("runfor: Runs a no-arg command for the specified number of times. Only used for sillyness.", RGBA::GRAY);
	}
	else if (arg0 == "changefont")
	{
		Console::instance->PrintLine("changefont: Changes the console's default font to a named font from the font folder.", RGBA::GRAY);
	}
	else
	{
		Console::instance->PrintLine("Undocumented or Unknown command", RGBA::GRAY);
	}
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(clear)
{
	UNUSED(args)
	Console::instance->ClearConsoleHistory();
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(quit)
{
	UNUSED(args)
	Console::instance->PrintLine("Saving and shutting down...", RGBA::RED);
	g_isQuitting = true;
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(runfor)
{
	if (!args.HasArgs(2))
	{
		Console::instance->PrintLine("runfor <# of Times to Run> <command name>", RGBA::GRAY);
		return;
	}
	int numberOfTimesToRun = args.GetIntArgument(0);
	std::string commandName = args.GetStringArgument(1);
	for (int i = 0; i < numberOfTimesToRun; i++)
	{
		Console::instance->RunCommand(commandName);
	}
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(changefont)
{
	if (!args.HasArgs(1))
	{
		Console::instance->PrintLine("changefont <fontName>", RGBA::GRAY);
		return;
	}
	std::string fontName = args.GetStringArgument(0);
	BitmapFont* font = BitmapFont::CreateOrGetFontFromGlyphSheet(fontName);
	if (font)
	{
		Console::instance->m_font = font;
		Console::instance->PrintLine(fontName + " successfully loaded!", RGBA::FOREST_GREEN);
	}
	else
	{
		Console::instance->PrintLine("Font not found", RGBA::MAROON);
	}
}