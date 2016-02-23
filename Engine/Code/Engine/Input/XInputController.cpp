#include "Engine/Input/XInputController.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vector2.hpp"
#include <cmath>

const float XInputController::DEADZONE = 0.153f;

//-----------------------------------------------------------------------------------
XInputController::XInputController(int controllerNumber) : m_controllerNumber(controllerNumber), m_isConnected(false), m_secondsToVibrate(0.f)
{
	memset(&m_state, 0, sizeof(m_state));
	memset(&m_previousState, 0, sizeof(m_previousState));
}

//-----------------------------------------------------------------------------------
XInputController::XInputController() : m_controllerNumber(INVALID_CONTROLLER_NUMBER), m_isConnected(false), m_secondsToVibrate(0.f)
{
	memset(&m_state, 0, sizeof(m_state));
	memset(&m_previousState, 0, sizeof(m_previousState));
}

//-----------------------------------------------------------------------------------
void XInputController::Update(float deltaTime)
{
	if (m_controllerNumber != INVALID_CONTROLLER_NUMBER)
	{
		m_previousState = m_state;
		memset(&m_state, 0, sizeof(m_state));
		DWORD errorStatus = XInputGetState(m_controllerNumber, &m_state);
		if (errorStatus == ERROR_SUCCESS)
		{
			m_isConnected = true;
		}
		else
		{
			m_isConnected = false;
		}
		if (m_secondsToVibrate > 0)
		{
			m_secondsToVibrate -= deltaTime;
			if (m_secondsToVibrate <= 0.f)
			{
				Vibrate(0, 0);
			}
		}
	}
}

//-----------------------------------------------------------------------------------
bool XInputController::IsPressed(XboxButton button)
{
	return IsPressed(m_state, button);
}

//-----------------------------------------------------------------------------------
bool XInputController::IsPressed(XINPUT_STATE &state, XboxButton button)
{
	WORD buttonState = state.Gamepad.wButtons;
	switch (button)
	{
	case XboxButton::DUP:
		return ((buttonState & XINPUT_GAMEPAD_DPAD_UP) != 0 ? true : false);
	case XboxButton::DDOWN:
		return ((buttonState & XINPUT_GAMEPAD_DPAD_DOWN) != 0 ? true : false);
	case XboxButton::DLEFT:
		return ((buttonState & XINPUT_GAMEPAD_DPAD_LEFT) != 0 ? true : false);
	case XboxButton::DRIGHT:
		return ((buttonState & XINPUT_GAMEPAD_DPAD_RIGHT) != 0 ? true : false);
	case XboxButton::START:
		return ((buttonState & XINPUT_GAMEPAD_START) != 0 ? true : false);
	case XboxButton::BACK:
		return ((buttonState & XINPUT_GAMEPAD_BACK) != 0 ? true : false);
	case XboxButton::LT:
		return ((buttonState & XINPUT_GAMEPAD_LEFT_THUMB) != 0 ? true : false);
	case XboxButton::RT:
		return ((buttonState & XINPUT_GAMEPAD_RIGHT_THUMB) != 0 ? true : false);
	case XboxButton::LB:
		return ((buttonState & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0 ? true : false);
	case XboxButton::RB:
		return ((buttonState & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0 ? true : false);
	case XboxButton::A: 
		return ((buttonState & XINPUT_GAMEPAD_A) != 0 ? true : false);
	case XboxButton::B:
		return ((buttonState & XINPUT_GAMEPAD_B) != 0 ? true : false);
	case XboxButton::X:
		return ((buttonState & XINPUT_GAMEPAD_X) != 0 ? true : false);
	case XboxButton::Y:
		return ((buttonState & XINPUT_GAMEPAD_Y) != 0 ? true : false);
	case XboxButton::numXboxButtons:
		return false;
	default:
		return false;
	}
	
}

//-----------------------------------------------------------------------------------
bool XInputController::JustPressed(XboxButton button)
{
	return !IsPressed(m_previousState, button) && IsPressed(m_state, button);
}

//-----------------------------------------------------------------------------------
bool XInputController::IsConnected()
{
	return m_isConnected;
}

//-----------------------------------------------------------------------------------
int XInputController::GetControllerNumber()
{
	return m_controllerNumber;
}

//-----------------------------------------------------------------------------------
Vector2 XInputController::GetLeftStickPosition()
{
	float X = (float)m_state.Gamepad.sThumbLX / 32768.f;
	float Y = (float)m_state.Gamepad.sThumbLY / 32768.f;
	return CalculateNormalizedStickPosition(X, Y);
}

//-----------------------------------------------------------------------------------
Vector2 XInputController::GetRightStickPosition()
{
	float X = (float)m_state.Gamepad.sThumbRX / 32768.f;
	float Y = (float)m_state.Gamepad.sThumbRY / 32768.f;
	return CalculateNormalizedStickPosition(X, Y);
}

//-----------------------------------------------------------------------------------
unsigned char XInputController::GetLeftTrigger()
{
	return static_cast<unsigned char>(m_state.Gamepad.bLeftTrigger);
}

//-----------------------------------------------------------------------------------
unsigned char XInputController::GetRightTrigger()
{
	return static_cast<unsigned char>(m_state.Gamepad.bRightTrigger);
}

//-----------------------------------------------------------------------------------
Vector2 XInputController::CalculateNormalizedStickPosition(float x, float y)
{
	float r = sqrt((x*x) + (y*y));
	float radians = atan2(y, x);
	r = MathUtils::RangeMap(r, 0.153f, 0.98f, 0.f, 1.f);
	r = MathUtils::Clamp(r, 0.0f, 1.0f);
	return Vector2(r * cos(radians), r * sin(radians));
}

//-----------------------------------------------------------------------------------
//Left motor is the heavy-duty motor, creates large vibrations.
//Right motor is for finer and more gentle vibrations.
void XInputController::Vibrate(int leftMotorVibration, int rightMotorVibration)
{
	XINPUT_VIBRATION vibration;
	ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
	vibration.wLeftMotorSpeed = (WORD)leftMotorVibration; //Use any value between 0-65535 here
	vibration.wRightMotorSpeed = (WORD)rightMotorVibration; //Use any value between 0-65535 here
	XInputSetState(m_controllerNumber, &vibration);
}

//-----------------------------------------------------------------------------------
void XInputController::VibrateForSeconds(float seconds, int leftMotorVibration, int rightMotorVibration)
{
	if ( m_secondsToVibrate < seconds )
		m_secondsToVibrate = seconds;
	Vibrate(leftMotorVibration, rightMotorVibration);
}
