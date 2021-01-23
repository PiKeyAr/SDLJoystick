#include "Common.h"

int Extension::GetAxis(int joy, int axis) const
{
	return DeviceIDOK(joy) && AxisIDOK(axis) ? (SDL_Data[joy].axis[axis]) : 0;
}

int Extension::GetButtonPressedState(int joy, int button) const
{
	return DeviceIDOK(joy) && ButtonIDOK(button) ? SDL_Data[joy].held_buttons[button] : 0;
}

int Extension::GetBallXDelta(int joy, int ball) const
{
	return DeviceIDOK(joy) && BallIDOK(ball) ? SDL_Data[joy].ball_x[ball] : 0;
}
int Extension::GetHat(int joy, int hat) const
{
	return DeviceIDOK(joy) && HatIDOK(hat) ? SDL_Data[joy].hat[hat] : 0;
}
int Extension::GetBallYDelta(int joy, int ball) const
{
	return DeviceIDOK(joy) && BallIDOK(ball) ? SDL_Data[joy].ball_y[ball] : 0;
}
int Extension::NumAxes(int joy) const
{
	return DeviceIDOK(joy) ? SDL_Data[joy].num_axes : 0;
}
int Extension::NumButtons(int joy) const
{
	return DeviceIDOK(joy) ? SDL_Data[joy].num_axes : 0;
}
int Extension::NumHats(int joy) const
{
	return DeviceIDOK(joy) ? SDL_Data[joy].num_hats : 0;
}
int Extension::NumBalls(int joy) const
{
	return DeviceIDOK(joy) ? SDL_Data[joy].num_balls : 0;
}
int Extension::NumJoysticks() const
{
	int numjoys = 0;
	for (int joy = 0; joy < NUM_DEVICES; ++joy)
		if (SDL_Data[joy].connected)
			++numjoys;

	return numjoys;
}
const TCHAR * Extension::GetDeviceGUID(int joy)
{
	if (!DeviceIDOK(joy) || !SDL_Data[joy].connected)
		return Runtime.CopyString(_T(""));

	char guidString[33];
	SDL_JoystickGUID guid = SDL_JoystickGetGUID(SDL_Data[joy].joystick);
	SDL_JoystickGetGUIDString(guid, guidString, 33);

	return Runtime.CopyString(UTF8ToTString(guidString).c_str());
}
const TCHAR * Extension::GetDeviceName(int joy)
{
	if (!DeviceIDOK(joy) || !SDL_Data[joy].connected)
		return Runtime.CopyString(_T(""));

	const std::string joyName = SDL_JoystickName(SDL_Data[joy].joystick);
	return Runtime.CopyString(UTF8ToTString(joyName).c_str());
}
const TCHAR * Extension::GetButtonsHeldHex(int joy)
{
	if (!DeviceIDOK(joy) || !SDL_Data[joy].connected)
		return Runtime.CopyString(_T(""));
	
	int buttons = 0;
	for (int i = 0; i < NUM_DEVICES; i++)
		if (SDL_Data[joy].held_buttons[i])
			buttons |= (1 << i);

	TCHAR res[17] = {};
	_stprintf(res, _T("%08X"), buttons);
	return Runtime.CopyString(res);
}
int Extension::HeldButtonIndex(int joy, int index) const
{
	return DeviceIDOK(joy) ? SDL_Data[joy].currentheld[index] : 0;
}

int Extension::LastButtonPressedIndex(int joy) const
{
	return DeviceIDOK(joy) ? SDL_Data[joy].lastpressed : 0;
}

int Extension::LastButtonReleasedIndex(int joy) const
{
	return DeviceIDOK(joy) ? SDL_Data[joy].lastreleased : 0;
}
