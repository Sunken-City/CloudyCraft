#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/XInputController.hpp"
#include "Engine/Math/Vector2Int.hpp"

InputSystem* InputSystem::instance = nullptr;

//-----------------------------------------------------------------------------------
InputSystem::InputSystem(void* hWnd, int maximumNumberOfControllers /*= 0*/)
: m_frameCounter(0)
, m_cursorDelta(0, 0)
, m_cursorPosition(0, 0)
, m_isCursorVisible(false)
, m_hasFocus(false)
, m_hWnd(hWnd)
, m_isScrolling(false)
, m_linesScrolled(0)
, m_maximumNumControllers(maximumNumberOfControllers)
, m_lastPressedChar(0x00) //NULL character
, m_captureCursor(false)
{
	//Only initialize the number of controllers we need for the game.
	for (int i = 0; i < m_maximumNumControllers; i++)
	{
		m_controllers[i] = new XInputController(i);
	}

	//Initialize all keys to up
	for (int keyIndex = 0; keyIndex < NUM_KEYS; ++keyIndex)
	{
		m_isKeyDown[keyIndex] = false;
		m_frameNumberKeyLastChanged[keyIndex] = m_frameCounter;
	}

	//Initialize all mouse buttons to up
	for (int mouseButtonIndex = 0; mouseButtonIndex < NUM_MOUSE_BUTTONS; ++mouseButtonIndex)
	{
		m_isMouseDown[mouseButtonIndex] = false;
		m_frameNumberMouseButtonLastChanged[mouseButtonIndex] = m_frameCounter;
	}
	ShowCursor(TRUE);
}

//-----------------------------------------------------------------------------------
InputSystem::~InputSystem()
{
	for (int i = 0; i < m_maximumNumControllers; i++)
	{
		delete m_controllers[i];
	}
}

//-----------------------------------------------------------------------------------
void InputSystem::Update(float deltaTime)
{
	for (int i = 0; i < m_maximumNumControllers; i++)
	{
		m_controllers[i]->Update(deltaTime);
	}

	HWND hWnd = static_cast<HWND>(m_hWnd);
	m_hasFocus = hWnd == GetFocus();

	if (m_hasFocus && m_captureCursor)
	{
		POINT cursorPos;
		BOOL success = GetCursorPos(&cursorPos);
		if (success)
		{
			m_cursorPosition = Vector2Int(cursorPos.x, cursorPos.y);
			m_cursorDelta.x = cursorPos.x - SNAP_BACK_X;
			m_cursorDelta.y = cursorPos.y - SNAP_BACK_Y;
			if (!m_isCursorVisible)
			{
				SetCursorPos(SNAP_BACK_X, SNAP_BACK_Y);
			}
		}
	}
}

//-----------------------------------------------------------------------------------
bool InputSystem::IsKeyDown(unsigned char keyCode)
{
	return m_isKeyDown[keyCode];
}

//-----------------------------------------------------------------------------------
bool InputSystem::WasKeyJustPressed(unsigned char keyCode)
{
	return (m_isKeyDown[keyCode] && (m_frameNumberKeyLastChanged[keyCode] == m_frameCounter));
}

//-----------------------------------------------------------------------------------
bool InputSystem::IsMouseButtonDown(unsigned char mouseButtonCode)
{
	return m_isMouseDown[mouseButtonCode];
}

//-----------------------------------------------------------------------------------
bool InputSystem::WasMouseButtonJustPressed(unsigned char mouseButtonCode)
{
	return (m_isMouseDown[mouseButtonCode] && (m_frameNumberMouseButtonLastChanged[mouseButtonCode] == m_frameCounter));
}

//-----------------------------------------------------------------------------------
bool InputSystem::IsScrolling()
{
	return m_isScrolling;
}

//-----------------------------------------------------------------------------------
int InputSystem::GetScrollAmountThisFrame()
{
	return m_linesScrolled;
}

//-----------------------------------------------------------------------------------
void InputSystem::HideMouseCursor()
{
	ShowCursor(FALSE);
	m_isCursorVisible = false;
}

//-----------------------------------------------------------------------------------
void InputSystem::ShowMouseCursor()
{
	ShowCursor(TRUE);
	m_isCursorVisible = true;
}

//-----------------------------------------------------------------------------------
void InputSystem::LockMouseCursor()
{
	m_captureCursor = true;
}

//-----------------------------------------------------------------------------------
void InputSystem::UnlockMouseCursor()
{
	m_captureCursor = false;
}

//-----------------------------------------------------------------------------------
void InputSystem::CaptureMouseCursor()
{
	HideMouseCursor();
	LockMouseCursor();
}

//-----------------------------------------------------------------------------------
void InputSystem::ReleaseMouseCursor()
{
	ShowMouseCursor();
	UnlockMouseCursor();
}

//-----------------------------------------------------------------------------------
Vector2Int InputSystem::GetDeltaMouse()
{
	return m_cursorDelta;
}

//-----------------------------------------------------------------------------------
Vector2Int InputSystem::GetMousePos()
{
	return m_cursorPosition;
}

//-----------------------------------------------------------------------------------
void InputSystem::SetLastPressedChar(unsigned char asKey)
{
	m_lastPressedChar = asKey;
}

//-----------------------------------------------------------------------------------
char InputSystem::GetLastPressedChar()
{
	return m_lastPressedChar;
}

//-----------------------------------------------------------------------------------
void InputSystem::SetCursorPosition(Vector2Int newPosition)
{
	SetCursorPos(newPosition.x, newPosition.y);
}

//-----------------------------------------------------------------------------------
bool InputSystem::HasFocus()
{
	return m_hasFocus;
}

//-----------------------------------------------------------------------------------
void InputSystem::SetMouseWheelStatus(short deltaMouseWheel)
{
	int MOUSE_WHEEL_SCROLL_AMOUNT_PER_LINE = 120;
	m_isScrolling = true;
	m_linesScrolled = deltaMouseWheel / MOUSE_WHEEL_SCROLL_AMOUNT_PER_LINE;
}

//-----------------------------------------------------------------------------------
void InputSystem::AdvanceFrameNumber()
{
	m_frameCounter++;
	m_isScrolling = false;
	m_linesScrolled = 0;
	m_lastPressedChar = 0x00;
}

//-----------------------------------------------------------------------------------
void InputSystem::SetKeyDownStatus(unsigned char keyCode, bool isNowDown)
{
	//If we are getting a keyboard repeat, ignore it when updating "just pressed" values.
	if (m_isKeyDown[keyCode] != isNowDown)
	{
		m_frameNumberKeyLastChanged[keyCode] = m_frameCounter;
	}
	m_isKeyDown[keyCode] = isNowDown;
}

//-----------------------------------------------------------------------------------
void InputSystem::SetMouseDownStatus(unsigned char mouseButton, bool isNowDown)
{
	//If we are getting a keyboard repeat, ignore it when updating "just pressed" values.
	if (m_isMouseDown[mouseButton] != isNowDown)
	{
		m_frameNumberMouseButtonLastChanged[mouseButton] = m_frameCounter;
	}
	m_isMouseDown[mouseButton] = isNowDown;
}
