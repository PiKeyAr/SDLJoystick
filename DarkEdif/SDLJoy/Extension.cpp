#include "Common.h"
#include <thread>

///
/// EXTENSION CONSTRUCTOR/DESTRUCTOR
///

Extension::Extension(RUNDATA * _rdPtr, EDITDATA * edPtr, CreateObjectInfo * cobPtr)
	: rdPtr(_rdPtr), rhPtr(_rdPtr->rHo.AdRunHeader), Runtime(_rdPtr), FusionDebugger(this)
{
	/*
		Link all your action/condition/expression functions to their IDs to match the
		IDs in the JSON here
	*/
	LinkAction(0, AddMappingsFromFile);

	LinkCondition(0, IsConnected);
	LinkCondition(1, ButtonPressed);
	LinkCondition(2, ButtonHeldDown);
	LinkCondition(3, ButtonReleased);
	LinkCondition(4, AnyButtonPressed);
	LinkCondition(5, AnyButtonHeldDown);
	LinkCondition(6, AnyButtonReleased);

	LinkExpression(0, NumJoysticks);
	LinkExpression(1, GetAxis);
	LinkExpression(2, GetButtonPressedState);
	LinkExpression(3, GetHat);
	LinkExpression(4, GetBallXDelta);
	LinkExpression(5, GetBallYDelta);
	LinkExpression(6, NumAxes);
	LinkExpression(7, NumButtons);
	LinkExpression(8, NumHats);
	LinkExpression(9, NumBalls);
	LinkExpression(10, GetDeviceName);
	LinkExpression(11, GetDeviceGUID);
	LinkExpression(12, GetButtonsHeldHex);
	LinkExpression(13, HeldButtonIndex);
	LinkExpression(14, LastButtonPressedIndex);
	LinkExpression(15, LastButtonReleasedIndex);

	/*
		This is where you'd do anything you'd do in CreateRunObject in the original SDK

		It's the only place you'll get access to edPtr at runtime, so you should transfer
		anything from edPtr to the extension class here.

	*/

	FusionDebugger.AddItemToDebugger(
		// reader function for your debug item
		[](Extension *ext, std::tstring &writeTo) {
			writeTo = _T("Number of joysticks: ") + std::to_tstring(ext->NumJoysticks());
		},
		NULL, 500, NULL
	);

	std::thread([this]()
	{
		// Open SDL library - pulled from SDL's testgamecontroller project
		int init = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC);
		char ErrorMsg[400];
		if (init != 0)
		{
			sprintf_s(ErrorMsg, "Error initializing SDL library: %s.", SDL_GetError());
			MessageBoxA(nullptr, ErrorMsg, "SDL Init Error", MB_OK | MB_ICONERROR);
			return;
		}

		sdlInited = true;
		if ((init = SDL_JoystickEventState(SDL_ENABLE)) < 1)
		{
			sprintf_s(ErrorMsg, "Error enabling SDL joystick events: %s.", SDL_GetError());
			MessageBoxA(nullptr, ErrorMsg, PROJECT_NAME " - SDL Init Error", MB_OK | MB_ICONERROR);
			return;
		}

#ifndef _DEBUG
#define WriteToLogIfDebug(...) ((void)0)
#else
#define WriteToLogIfDebug(...) SDL_Log(__VA_ARGS__)

		/* Enable standard application logging */
		SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

		int i;
		int nController = 0;
		int retcode = 0;
		char guid[64];
		/* Print information about the controller */
		for (i = 0; i < SDL_NumJoysticks(); ++i) {
			const char * name;
			const char * description;

			SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(i),
				guid, sizeof(guid));

			if (SDL_IsGameController(i))
			{
				nController++;
				name = SDL_GameControllerNameForIndex(i);
				switch (SDL_GameControllerTypeForIndex(i)) {
				case SDL_CONTROLLER_TYPE_XBOX360:
					description = "XBox 360 Controller";
					break;
				case SDL_CONTROLLER_TYPE_XBOXONE:
					description = "XBox One Controller";
					break;
				case SDL_CONTROLLER_TYPE_PS3:
					description = "PS3 Controller";
					break;
				case SDL_CONTROLLER_TYPE_PS4:
					description = "PS4 Controller";
					break;
				case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO:
					description = "Nintendo Switch Pro Controller";
					break;
				default:
					description = "Game Controller";
					break;
				}
			}
			else {
				name = SDL_JoystickNameForIndex(i);
				description = "Joystick";
			}
			SDL_Log("%s %d: %s (guid %s, VID 0x%.4x, PID 0x%.4x, player index = %d)\n",
				description, i, name ? name : "Unknown", guid,
				SDL_JoystickGetDeviceVendor(i), SDL_JoystickGetDeviceProduct(i), SDL_JoystickGetDevicePlayerIndex(i));
		}
		SDL_Log("There are %d game controller(s) attached (%d joystick(s))\n", nController, SDL_NumJoysticks());
#endif
	}).detach(); // Run in secondary thread as of build 5
}

Extension::~Extension()
{
	if (sdlInited)
		SDL_Quit();
}

void Extension::CloseJoystick(int joy)
{
	for (int i = 0; i < 8; i++)
		SDL_Data[joy].axis[i] = 0;
	for (int i = 0; i < 32; i++) {
		SDL_Data[joy].held_buttons[i] = false;
		SDL_Data[joy].held_buttons_last[i] = false;
	}
	for (int i = 0; i < 4; i++)
		SDL_Data[joy].hat[i] = 0;
	for (int i = 0; i < 4; i++)
	{
		SDL_Data[joy].ball_x[i] = 0;
		SDL_Data[joy].ball_y[i] = 0;
	}
	SDL_Data[joy].joy_id = -1;
	SDL_Data[joy].num_axes = 0;
	SDL_Data[joy].num_buttons = 0;
	SDL_Data[joy].num_hats = 0;
	SDL_Data[joy].num_balls = 0;
	if (SDL_Data[joy].haptic != nullptr) {
		SDL_HapticClose(SDL_Data[joy].haptic);
		SDL_Data[joy].haptic = nullptr;
	}
	if (SDL_Data[joy].joystick != nullptr) {
		SDL_JoystickClose(SDL_Data[joy].joystick);
		SDL_Data[joy].joystick = nullptr;
	}
	SDL_Data[joy].connected = false;
}

void Extension::OpenJoystick(int joy, int which)
{
	// Already opened, close then reopen
	if (SDL_Data[joy].connected)
		CloseJoystick(joy);

	SDL_Data[joy].joystick = SDL_JoystickOpen(which);
	if (SDL_Data[joy].joystick == nullptr)
	{
		SDL_Data[joy].connected = false;
		SDL_Data[joy].joy_id = -1;
		return;
	}
	SDL_Data[joy].joy_id = SDL_JoystickInstanceID(SDL_Data[joy].joystick);
	SDL_Data[joy].num_axes = SDL_JoystickNumAxes(SDL_Data[joy].joystick);
	SDL_Data[joy].num_buttons = SDL_JoystickNumButtons(SDL_Data[joy].joystick);
	SDL_Data[joy].num_hats = SDL_JoystickNumHats(SDL_Data[joy].joystick);
	SDL_Data[joy].num_balls = SDL_JoystickNumBalls(SDL_Data[joy].joystick);
	SDL_Data[joy].haptic = SDL_HapticOpenFromJoystick(SDL_Data[joy].joystick);
	SDL_Data[joy].lastpressed = -1;
	SDL_Data[joy].lastreleased = -1;
	for (int h = 0; h < 32; h++)
	{
		SDL_Data[joy].currentheld[h] = -1;
	}
	if (SDL_Data[joy].haptic == nullptr) {
		SDL_Data[joy].connected = true;
		return;
	}
	if (!SDL_HapticRumbleSupported(SDL_Data[joy].haptic)) {
		SDL_HapticClose(SDL_Data[joy].haptic);
		SDL_Data[joy].haptic = nullptr;
	}
	SDL_Data[joy].connected = true;
}

REFLAG Extension::Handle()
{
	if (!sdlInited)
		return REFLAG::NONE;

	// Skip first few ticks of frame to prevent SDL_PollEvent stuttering the app
	// while event system loads
	if (tickCount < 5)
	{
		++tickCount;
		return REFLAG::NONE;
	}

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		//Check for device events
		switch (event.jdevice.type)
		{
			case SDL_JOYDEVICEADDED:
			{
				const int which = event.jdevice.which;
				WriteToLogIfDebug("Device added; which: %d", which);
				for (int joy = 0; joy < 16; joy++)
				{
					if (!SDL_Data[joy].connected)
					{
						OpenJoystick(joy, which);
						break;
					}
				}
				break;
			}
			case SDL_JOYDEVICEREMOVED:
			{
				const int which = event.jdevice.which;
				WriteToLogIfDebug("Device removed; which: %d", which);
				for (int joy = 0; joy < 16; joy++)
				{
					if (SDL_JoystickInstanceID(SDL_Data[joy].joystick) == which)
					{
						WriteToLogIfDebug("Device removed; which %d, joy %d.", which, joy);
						CloseJoystick(joy);
						break;
					}
				}
				break;
			}
			default:
				break;
		}
	}

	// Update input
	for (size_t i = 0; i < std::size(SDL_Data); i++)
	{
		if (!SDL_Data[i].connected)
			continue;

		// Axes
		for (int a = 0; a < SDL_Data[i].num_axes; a++)
			SDL_Data[i].axis[a] = SDL_JoystickGetAxis(SDL_Data[i].joystick, a);
		// Buttons
		for (int b = 0; b < SDL_Data[i].num_buttons; b++)
		{
			SDL_Data[i].held_buttons_last[b] = SDL_Data[i].held_buttons[b];
			SDL_Data[i].held_buttons[b] = SDL_JoystickGetButton(SDL_Data[i].joystick, b);
		}
		// Buttons released
		for (int b = 0; b < SDL_Data[i].num_buttons; b++)
		{
			if (!SDL_Data[i].held_buttons[b])
			{
				for (int hh2 = 0; hh2 < 32; hh2++)
				{
					if (SDL_Data[i].currentheld[hh2] == b)
					{
						SDL_Data[i].currentheld[hh2] = -1;
						break;
					}
				}
			}
		}
		// Buttons held
		for (int b = 0; b < SDL_Data[i].num_buttons; b++)
		{
			if (SDL_Data[i].held_buttons[b])
			{
				for (int hh2 = 0; hh2 < 32; hh2++)
				{
					if (SDL_Data[i].currentheld[hh2] == b) break;
					if (SDL_Data[i].currentheld[hh2] == -1)
					{
						SDL_Data[i].currentheld[hh2] = b;
						break;
					}
				}
			}
		}
		// Hats
		for (int h = 0; h < SDL_Data[i].num_hats; h++)
			SDL_Data[i].hat[h] = SDL_JoystickGetHat(SDL_Data[i].joystick, h);
		// Balls
		for (int l = 0; l < SDL_Data[i].num_balls; l++)
			SDL_JoystickGetBall(SDL_Data[i].joystick, l, &SDL_Data[i].ball_x[l], &SDL_Data[i].ball_y[l]);
	}
	return REFLAG::NONE;
}


REFLAG Extension::Display()
{
	/*
		If you return REFLAG_DISPLAY in Handle() this routine will run.
	*/

	// Ok
	return REFLAG::DISPLAY;
}

short Extension::Pause()
{

	// Ok
	return 0;
}

short Extension::Continue()
{

	// Ok
	return 0;
}


// These are called if there's no function linked to an ID

void Extension::Action(int ID, RUNDATA * rdPtr, long param1, long param2)
{

}

long Extension::Condition(int ID, RUNDATA * rdPtr, long param1, long param2)
{
	return false;
}

long Extension::Expression(int ID, RUNDATA * rdPtr, long param)
{
	return 0;
}


