#pragma once

#ifdef WIN32
#define PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include <Xinput.h> // include the Xinput API
#pragma comment( lib, "xinput9_1_0" ) // Link in the xinput.lib static library // #Eiserloh: Xinput 1_4 doesn't work in Windows 7; using 9_1_0 explicitly

class Vector2;

//ENUMS//////////////////////////////////////////////////////////////////////////
enum class XboxButton
{
	DUP,
	DDOWN,
	DLEFT,
	DRIGHT,
	A,
	B,
	X,
	Y,
	LB,
	RB,
	LT,
	RT,
	START,
	BACK,
	HOME,
	numXboxButtons
};


class XInputController
{
public:
	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
	XInputController();
	XInputController(int controllerNumber);
	
	//FUNCTIONS//////////////////////////////////////////////////////////////////////////
	void Update(float deltaTime);
	void Vibrate(int leftMotorVibration, int rightMotorVibration);
	void VibrateForSeconds(float seconds, int leftMotorVibration, int rightMotorVibration);

	//QUERIES//////////////////////////////////////////////////////////////////////////
	bool IsPressed(XboxButton button);
	bool IsConnected();
	bool JustPressed(XboxButton button);

	//GETTERS//////////////////////////////////////////////////////////////////////////
	int GetControllerNumber();
	Vector2 GetLeftStickPosition();
	Vector2 GetRightStickPosition();
	unsigned char GetLeftTrigger();
	unsigned char GetRightTrigger();

	//CONSTANTS//////////////////////////////////////////////////////////////////////////
	static const int MAX_CONTROLLERS = 4;
	static const int MAX_VIBRATE = 65535;
	static const int LARGE_VIBRATE = 48000;
	static const int MEDIUM_VIBRATE = 32000;
	static const int LOW_VIBRATE = 16000;
	static const int EXTREMELY_LOW_VIBRATE = 8000;
	static const int NO_VIBRATE = 0;
	static const float DEADZONE;

private:
	static const int INVALID_CONTROLLER_NUMBER = -1;

	//HELPER FUNCTIONS//////////////////////////////////////////////////////////////////////////
	bool IsPressed(XINPUT_STATE &state, XboxButton button);
	Vector2 CalculateNormalizedStickPosition(float x, float y);

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	XINPUT_STATE m_state;
	XINPUT_STATE m_previousState;
	int m_controllerNumber;
	bool m_isConnected;
	float m_secondsToVibrate;
};