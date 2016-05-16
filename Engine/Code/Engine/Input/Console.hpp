#pragma once
#include <vector>
#include <map>
#include <string>
#include "Engine\Renderer\RGBA.hpp"
//-----------------------------------------------------------------------------------------------
#define UNUSED(x) (void)(x);

class Command;
class BitmapFont;
typedef void(*ConsoleCommandFunctionPointer)(Command&);

//Used for quitting the application, bound to our Main_Win32.cpp; remove this if we aren't using it anymore.
extern bool g_isQuitting;
extern std::map<std::string, ConsoleCommandFunctionPointer>* g_consoleCommands;

//----------------------------------------------------------------------------------------------
struct ColoredText
{
	ColoredText(std::string line, RGBA tint) : text(line), color(tint) {};
	std::string text;
	RGBA color;
};

//----------------------------------------------------------------------------------------------
class Console
{
public:
	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
	Console();
	~Console();

	//FUNCTIONS//////////////////////////////////////////////////////////////////////////
	void Update(float deltaSeconds);
	void Render() const;
	void ParseKey(char currentChar);
	void ToggleConsole();
	void ActivateConsole();
	void DeactivateConsole(); 
	void ClearConsoleHistory();
	void PrintLine(std::string consoleLine, RGBA color = RGBA::WHITE);
	bool RunCommand(std::string commandLine);
	inline bool IsActive() { return m_isActive; };
	inline bool IsEmpty() { return (m_cursorPointer == m_currentLine && *m_cursorPointer == '\0'); };
	static void RegisterCommand(std::string commandName, ConsoleCommandFunctionPointer consoleFunction);

	//VARIABLES//////////////////////////////////////////////////////////////////////////
	static Console* instance;
	BitmapFont* m_font;

private:
	//CONSTANTS//////////////////////////////////////////////////////////////////////////
	static const int MAX_LINE_LENGTH = 128;
	static const int MAX_CONSOLE_LINES = 30;
	static const char CURSOR_CHARACTER;
	static const float CHARACTER_HEIGHT;
	static const float CHARACTER_WIDTH;
	static const float CURSOR_BLINK_RATE_SECONDS;

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	bool m_isActive;
	bool m_isCursorShowing;
	char* m_currentLine;
	char* m_cursorPointer;
	char m_characterAtCursor;
	float m_timeSinceCursorBlink;
	std::vector<ColoredText> m_consoleHistory;
};

//----------------------------------------------------------------------------------------------
class Command
{
public:
	Command(std::string fullCommandStr); //Split name and args into two buffers
	inline std::string GetCommandName() const { return m_commandName; };
	inline bool HasArgs(int argNumber) const { return m_argsList.size() == (unsigned int)argNumber; };
	inline std::string GetStringArgument(int argNumber) const { return m_argsList[argNumber]; };
	inline int GetIntArgument(int argNumber) const { return std::stoi(m_argsList[argNumber]); };
	float GetFloatArgument(int argNumber) const { return std::stof(m_argsList[argNumber]); };
private:
	const std::string m_fullCommandStr;
	std::string m_commandName;
	std::vector<std::string> m_argsList;
};

//----------------------------------------------------------------------------------------------
class RegisterCommandHelper
{
public:
	RegisterCommandHelper(std::string name, ConsoleCommandFunctionPointer command)
	{
		if (!g_consoleCommands)
		{
			g_consoleCommands = new std::map<std::string, ConsoleCommandFunctionPointer>();
		}
		Console::RegisterCommand(name, command);
	}
};

//Macro that allows us to define a console command from anywhere
#define CONSOLE_COMMAND(name) void ConsoleCommand_ ## name ## ( Command & args ); \
	static RegisterCommandHelper RegistrationHelper_ ## name ##( #name, ConsoleCommand_ ## name ## ); \
	void ConsoleCommand_ ## name ##(Command &args)
