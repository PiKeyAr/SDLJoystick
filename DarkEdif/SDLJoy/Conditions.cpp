#include "Common.h"

bool Extension::DeviceIDOK(int devID) const
{
	return (devID >= 0 || devID <= NUM_DEVICES - 1);
}
bool Extension::ButtonIDOK(int buttonID) const
{
	return (buttonID >= 0 || buttonID <= NUM_BUTTONS - 1);
}
bool Extension::HatIDOK(int buttonID) const
{
	return (buttonID >= 0 || buttonID <= NUM_HATS - 1);
}
bool Extension::AxisIDOK(int axisID) const
{
	return (axisID >= 0 || axisID <= NUM_AXES - 1);
}
bool Extension::BallIDOK(int ballID) const {
	return (ballID >= 0 || ballID <= NUM_BALLS - 1);
}

bool Extension::IsConnected(int devID) const
{
	return DeviceIDOK(devID) && SDL_Data[devID].connected;
}
bool Extension::ButtonPressed(int devID, int buttonID) const
{
	return DeviceIDOK(devID) && ButtonIDOK(buttonID) && (SDL_Data[devID].held_buttons[buttonID] && !SDL_Data[devID].held_buttons_last[buttonID]);
}
bool Extension::ButtonHeldDown(int devID, int buttonID) const
{
	return DeviceIDOK(devID) && ButtonIDOK(buttonID) && SDL_Data[devID].held_buttons[buttonID];
}
bool Extension::ButtonReleased(int devID, int buttonID) const
{
	return DeviceIDOK(devID) && ButtonIDOK(buttonID) && (!SDL_Data[devID].held_buttons[buttonID] && SDL_Data[devID].held_buttons_last[buttonID]);
}
bool Extension::AnyButtonPressed(int devID) const
{
	if (!DeviceIDOK(devID))
		return false;
	for (int i = 0; i < NUM_BUTTONS; i++)
		if (SDL_Data[devID].held_buttons[i] && !SDL_Data[devID].held_buttons_last[i])
			return true;
	return false;
}
bool Extension::AnyButtonHeldDown(int devID) const
{
	if (!DeviceIDOK(devID))
		return false;
	for (int i = 0; i < NUM_BUTTONS; i++)
		if (SDL_Data[devID].held_buttons[i])
			return true;
	return false;
}
bool Extension::AnyButtonReleased(int devID) const
{
	if (!DeviceIDOK(devID))
		return false;
	for (int i = 0; i < NUM_BUTTONS; i++)
		if (!SDL_Data[devID].held_buttons[i] && SDL_Data[devID].held_buttons_last[i])
			return true;
	return false;
}
