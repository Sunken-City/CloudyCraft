#pragma once
#include "Engine/Math/Vector2Int.hpp"

class XInputController;

class InputSystem
{
public:
	InputSystem(void* hWnd);
	~InputSystem();

	void Update(float deltaTime);
	void AdvanceFrameNumber();
	void HideMouseCursor();
	void ShowMouseCursor();

	void SetCursorPosition(Vector2Int newPosition);
	void SetMouseWheelStatus(short deltaMouseWheel);
	void SetKeyDownStatus(unsigned char keyCode, bool isDown);
	void SetMouseDownStatus(unsigned char mouseButton, bool isNowDown);

	bool IsKeyDown(unsigned char keyCode);
	bool WasKeyJustPressed(unsigned char keyCode);
	bool IsMouseButtonDown(unsigned char mouseButtonCode);
	bool WasMouseButtonJustPressed(unsigned char mouseButtonCode);
	bool IsScrolling();
	bool HasFocus();

	int GetScrollAmountThisFrame();
	Vector2Int GetDeltaMouse();
	Vector2Int GetMousePos();

	static InputSystem* instance;
	XInputController* m_controllers[4];

	enum ExtraKeys
	{
		SHIFT = 0x10, //VK_SHIFT
		F5 = 0x74, //VK_F5
		NUM_EXTRA_KEYS
	};

private:
	static const int NUM_KEYS = 256;
	static const int NUM_MOUSE_BUTTONS = 3;
	static const int MAX_CONTROLLERS = 3;
	static const int SNAP_BACK_X = 800;
	static const int SNAP_BACK_Y = 600;

	bool m_isKeyDown[NUM_KEYS];
	bool m_isMouseDown[NUM_MOUSE_BUTTONS];
	bool m_isCursorVisible;
	bool m_hasFocus;
	bool m_isScrolling;
	int m_linesScrolled;
	int m_frameNumberKeyLastChanged[NUM_KEYS];
	int m_frameNumberMouseButtonLastChanged[NUM_MOUSE_BUTTONS];
	int m_frameCounter;
	Vector2Int m_cursorDelta;
	Vector2Int m_cursorPosition;
	void* m_hWnd;
};
