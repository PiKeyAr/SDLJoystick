// ============================================================================
//
// This file contains routines that are handled during the Runtime.
//
// Including creating, display, and handling your object.
// 
// ============================================================================

// Common Include
#include "common.h"
#include "SDL.h"

// --------------------
// GetRunObjectDataSize
// --------------------
// Returns the size of the runtime datazone of the object
// 
ushort WINAPI DLLExport GetRunObjectDataSize(fprh rhPtr, LPEDATA edPtr)
{
	return(sizeof(RUNDATA));
}

// ---------------
// CreateRunObject
// ---------------
// The routine where the object is actually created
// 

short WINAPI DLLExport CreateRunObject(LPRDATA rdPtr, LPEDATA edPtr, fpcob cobPtr)
{
	TCHAR SDLPath[400];
	TCHAR rootdir[MAX_PATH];
	TCHAR ErrorMsg[400];
	DWORD dwError = 0;
	// Reset joystick IDs and held buttons
	for (int i = 0; i < 16; i++)
	{
		rdPtr->SDL_Data[i].joy_id = -1;
		rdPtr->SDL_Data[i].lastpressed = -1;
		rdPtr->SDL_Data[i].lastreleased = -1;
		for (int h = 0; h < 32; h++)
		{
			rdPtr->SDL_Data[i].currentheld[h] = -1;
		}
	}
	// Get path to DLL
	GetCurrentDirectory(MAX_PATH - 1, rootdir);
	sprintf(SDLPath, "%s\\%s", rootdir, "SDL2.DLL");
	// Open SDL library
	rdPtr->SDL = LoadLibrary(SDLPath);
	dwError = GetLastError();
	if (rdPtr->SDL == nullptr)
	{
		sprintf(ErrorMsg, "Error loading library %s: code %d.", SDLPath, dwError);
		MessageBoxA(nullptr, ErrorMsg,
			"SDL Load Error", MB_OK | MB_ICONERROR);
		return 0;
	}
	int init;
	if ((init = SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC | SDL_INIT_EVENTS)) != 0)
	{
		sprintf(ErrorMsg, "Error initializing SDL: %s.", SDL_GetError());
		MessageBoxA(nullptr, ErrorMsg,
			"SDL Init Error", MB_OK | MB_ICONERROR);
		return 0;
	}
	SDL_JoystickEventState(SDL_ENABLE);
	return 0;
}

// ----------------
// DestroyRunObject
// ----------------
// Destroys the run-time object
// 
short WINAPI DLLExport DestroyRunObject(LPRDATA rdPtr, long fast)
{
	if (rdPtr->SDL != nullptr)
	{
		SDL_Quit();
		FreeLibrary(rdPtr->SDL);
	}
	// No errors
	return 0;
}

void CloseJoystick(LPRDATA rdPtr, int joy)
{
	for (int i = 0; i < 8; i++)
	{
		rdPtr->SDL_Data[joy].axis[i] = 0;
	}
	for (int i = 0; i < 32; i++)
	{
		rdPtr->SDL_Data[joy].held_buttons[i] = false;
		rdPtr->SDL_Data[joy].held_buttons_last[i] = false;
	}
	for (int i = 0; i < 4; i++)
	{
		rdPtr->SDL_Data[joy].hat[i] = 0;
	}
	for (int i = 0; i < 4; i++)
	{
		rdPtr->SDL_Data[joy].ball_x[i] = 0;
		rdPtr->SDL_Data[joy].ball_y[i] = 0;
	}
	rdPtr->SDL_Data[joy].joy_id = -1;
	rdPtr->SDL_Data[joy].num_axes = 0;
	rdPtr->SDL_Data[joy].num_buttons = 0;
	rdPtr->SDL_Data[joy].num_hats = 0;
	rdPtr->SDL_Data[joy].num_balls = 0;
	if (rdPtr->SDL_Data[joy].haptic != nullptr)
	{
		SDL_HapticClose(rdPtr->SDL_Data[joy].haptic);
		rdPtr->SDL_Data[joy].haptic = nullptr;
	}
	if (rdPtr->SDL_Data[joy].joystick != nullptr)
	{
		SDL_JoystickClose(rdPtr->SDL_Data[joy].joystick);
		rdPtr->SDL_Data[joy].joystick = nullptr;
	}
	rdPtr->SDL_Data[joy].connected = false;
}

void OpenJoystick(LPRDATA rdPtr, int joy, int which)
{
	if (rdPtr->SDL_Data[joy].connected) CloseJoystick(rdPtr, joy);
	rdPtr->SDL_Data[joy].joystick = SDL_JoystickOpen(which);
	if (rdPtr->SDL_Data[joy].joystick == nullptr)
	{
		rdPtr->SDL_Data[joy].connected = false;
		rdPtr->SDL_Data[joy].joy_id = -1;
		return;
	}
	rdPtr->SDL_Data[joy].joy_id = SDL_JoystickInstanceID(rdPtr->SDL_Data[joy].joystick);
	rdPtr->SDL_Data[joy].num_axes = SDL_JoystickNumAxes(rdPtr->SDL_Data[joy].joystick);
	rdPtr->SDL_Data[joy].num_buttons = SDL_JoystickNumButtons(rdPtr->SDL_Data[joy].joystick);
	rdPtr->SDL_Data[joy].num_hats = SDL_JoystickNumHats(rdPtr->SDL_Data[joy].joystick);
	rdPtr->SDL_Data[joy].num_balls = SDL_JoystickNumBalls(rdPtr->SDL_Data[joy].joystick);
	rdPtr->SDL_Data[joy].haptic = SDL_HapticOpenFromJoystick(rdPtr->SDL_Data[joy].joystick);
	rdPtr->SDL_Data[joy].lastpressed = -1;
	rdPtr->SDL_Data[joy].lastreleased = -1;
	for (int h = 0; h < 32; h++)
	{
		rdPtr->SDL_Data[joy].currentheld[h] = -1;
	}
	if (rdPtr->SDL_Data[joy].haptic == nullptr)
	{
		rdPtr->SDL_Data[joy].connected = true;
		return;
	}
	if (!SDL_HapticRumbleSupported(rdPtr->SDL_Data[joy].haptic))
	{
		SDL_HapticClose(rdPtr->SDL_Data[joy].haptic);
		rdPtr->SDL_Data[joy].haptic = nullptr;
	}
	rdPtr->SDL_Data[joy].connected = true;
}

// ----------------
// HandleRunObject
// ----------------
// Called (if you want) each loop, this routine makes the object live
// 
short WINAPI DLLExport HandleRunObject(LPRDATA rdPtr)
{
	//TCHAR ErrorMessage[400];
	if (rdPtr->SDL == nullptr) return REFLAG_DISPLAY;
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		// Check for device events
		switch (event.jdevice.type)
		{
		default:
			break;
		case SDL_JOYDEVICEADDED:
		{
			const int which = event.jdevice.which;
			//sprintf(ErrorMessage, "Device added; which: %d", which);
			//MessageBoxA(nullptr, ErrorMessage, "SDL message", MB_OK | MB_ICONINFORMATION);
			for (int joy = 0; joy < 16; joy++)
			{
				if (!rdPtr->SDL_Data[joy].connected)
				{
					OpenJoystick(rdPtr, joy, which);
					break;
				}
			}
			break;
		}
		case SDL_JOYDEVICEREMOVED:
		{
			const int which = event.jdevice.which;
			//sprintf(ErrorMessage, "Device removed; which: %d", which);
			//MessageBoxA(nullptr, ErrorMessage, "SDL message", MB_OK | MB_ICONINFORMATION);
			for (int joy = 0; joy < 16; joy++)
			{
				if (SDL_JoystickInstanceID(rdPtr->SDL_Data[joy].joystick) == which)
				{
					//MessageBoxA(nullptr, "Device removed.",	"SDL message", MB_OK | MB_ICONINFORMATION);
					CloseJoystick(rdPtr, joy);
					break;
				}
			}
			break;
		}
		}
	}
	// Update input
	for (int i = 0; i < 16; i++)
	{
		if (rdPtr->SDL_Data[i].connected)
		{
			// Axes
			for (int a = 0; a < rdPtr->SDL_Data[i].num_axes; a++)
			{
				rdPtr->SDL_Data[i].axis[a] = SDL_JoystickGetAxis(rdPtr->SDL_Data[i].joystick, a);
			}
			// Buttons pressed
			for (int b = 0; b < rdPtr->SDL_Data[i].num_buttons; b++)
			{
				rdPtr->SDL_Data[i].held_buttons_last[b] = rdPtr->SDL_Data[i].held_buttons[b];
				rdPtr->SDL_Data[i].held_buttons[b] = SDL_JoystickGetButton(rdPtr->SDL_Data[i].joystick, b);
			}
			// Buttons released
			for (int b = 0; b < rdPtr->SDL_Data[i].num_buttons; b++)
			{
				if (!rdPtr->SDL_Data[i].held_buttons[b])
				{
					for (int hh2 = 0; hh2 < 32; hh2++)
					{
						if (rdPtr->SDL_Data[i].currentheld[hh2] == b)
						{
							rdPtr->SDL_Data[i].currentheld[hh2] = -1;
							break;
						}
					}
				}
			}
			// Buttons held
			for (int b = 0; b < rdPtr->SDL_Data[i].num_buttons; b++)
			{
				if (rdPtr->SDL_Data[i].held_buttons[b])
				{
					for (int hh2 = 0; hh2 < 32; hh2++)
					{
						if (rdPtr->SDL_Data[i].currentheld[hh2] == b) break;
						if (rdPtr->SDL_Data[i].currentheld[hh2] == -1)
						{
							rdPtr->SDL_Data[i].currentheld[hh2] = b;
							break;
						}
					}
				}
			}
			// Buttons last pressed/released
			for (int b = 0; b < 32; b++)
			{
				if (rdPtr->SDL_Data[i].held_buttons[b])
				{
					if (!rdPtr->SDL_Data[i].held_buttons_last[b])
						rdPtr->SDL_Data[i].lastpressed = b;
				}
				else if (rdPtr->SDL_Data[i].held_buttons_last[b])
					rdPtr->SDL_Data[i].lastreleased = b;
			}
			// Hats
			for (int h = 0; h < rdPtr->SDL_Data[i].num_hats; h++)
			{
				rdPtr->SDL_Data[i].hat[h] = SDL_JoystickGetHat(rdPtr->SDL_Data[i].joystick, h);
			}
			// Balls
			for (int l = 0; l < rdPtr->SDL_Data[i].num_balls; l++)
			{
				SDL_JoystickGetBall(rdPtr->SDL_Data[i].joystick, l, &rdPtr->SDL_Data[i].ball_x[l], &rdPtr->SDL_Data[i].ball_y[l]);
			}
		}
	}
	return REFLAG_DISPLAY;
}

// ----------------
// DisplayRunObject
// ----------------
// Draw the object in the application screen.
// 
short WINAPI DLLExport DisplayRunObject(LPRDATA rdPtr)
{
	// Ok
	return 0;
}

// ----------------
// PauseRunObject
// ----------------
// Enters the pause mode
// 
short WINAPI DLLExport PauseRunObject(LPRDATA rdPtr)
{
	// Ok
	return 0;
}


// -----------------
// ContinueRunObject
// -----------------
// Quits the pause mode
// 
short WINAPI DLLExport ContinueRunObject(LPRDATA rdPtr)
{
	// Ok
	return 0;
}

// ============================================================================
//
// WINDOWPROC (interception of messages sent to hMainWin and hEditWin)
//
// Do not forget to enable the WindowProc function in the .def file if you implement it
// 
// ============================================================================

// Get the pointer to the object's data from its window handle
// Note: the object's window must have been subclassed with a
// callRunTimeFunction(rdPtr, RFUNCTION_SUBCLASSWINDOW, 0, 0);
// See the documentation and the Simple Control example for more info.
/*
LPRDATA GetRdPtr(HWND hwnd, LPRH rhPtr)
{
	return (LPRDATA)GetProp(hwnd, (LPCSTR)rhPtr->rh4.rh4AtomRd);
}

// Called from the window proc of hMainWin and hEditWin.
// You can intercept the messages and/or tell the main proc to ignore them.
//
/LRESULT CALLBACK DLLExport WindowProc(LPRH rhPtr, HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return 0;
}*/