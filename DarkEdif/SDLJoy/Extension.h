#include <DarkEdif.h>

#define SDL_MAIN_HANDLED
#include "sdl/SDL_config.h"
#include "sdl/SDL.h"
#include "sdl/SDL_main.h"
#include "sdl/SDL_joystick.h"
#include "sdl/SDL_haptic.h"

#pragma comment (lib, "Setupapi.lib")
#pragma comment (lib, "Version.lib")
#pragma comment (lib, "Imm32.lib")

#ifdef _DEBUG
#pragma comment (lib, "../Lib/Windows/sdl2d.lib")
#pragma comment (lib, "../Lib/Windows/sdl2maind.lib")
#else
#pragma comment (lib, "../Lib/Windows/sdl2.lib")
#pragma comment (lib, "../Lib/Windows/sdl2main.lib")
#endif

struct SDL_JoystickData
{
	SDL_Joystick * joystick = nullptr;
	SDL_Haptic * haptic = nullptr;
	bool connected = false;
	int joy_id = -1;
	Sint16 axis[8] = {};
	Uint8 held_buttons[32] = {};
	Uint8 held_buttons_last[32] = {};
	Uint8 hat[4] = {};
	int ball_x[4] = {};
	int ball_y[4] = {};
	Sint8 currentheld[32] = {
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	Sint8 lastpressed = -1;
	Sint8 lastreleased = -1;
	char num_buttons = 0;
	char num_axes = 0;
	char num_hats = 0;
	char num_balls = 0;
};


class Extension
{
public:

	RUNDATA * rdPtr;
	RunHeader *	rhPtr;

	Edif::Runtime Runtime;

	static const int MinimumBuild = 254;
	static const int Version = 5;

	static const OEFLAGS OEFLAGS = OEFLAGS::NEVER_SLEEP;
	static const OEPREFS OEPREFS = OEPREFS::NONE;

	static const int WindowProcPriority = 100;

	Extension(RUNDATA * rdPtr, EDITDATA * edPtr, CreateObjectInfo * cobPtr);
	~Extension();

	DarkEdif::FusionDebugger FusionDebugger;

	void CloseJoystick(int joy);
	void OpenJoystick(int joy, int which);

	constexpr static int NUM_HATS = 4;
	constexpr static int NUM_AXES = 8;
	constexpr static int NUM_BALLS = 8;
	constexpr static int NUM_DEVICES = 16;
	constexpr static int NUM_BUTTONS = 32;

	SDL_JoystickData SDL_Data[NUM_DEVICES];
	bool DeviceIDOK(int devID) const;
	bool ButtonIDOK(int buttonID) const;
	bool HatIDOK(int devID) const;
	bool AxisIDOK(int axisID) const;
	bool BallIDOK(int ballID) const;

	bool sdlInited = false;
	int tickCount = 0;

	// int MyVariable;




	/*  Add your actions, conditions and expressions as real class member
		functions here. The arguments (and return type for expressions) must
		match EXACTLY what you defined in the JSON.

		Remember to link the actions, conditions and expressions to their
		numeric IDs in the class constructor (Extension.cpp)
	*/

	/// Actions
		void AddMappingsFromFile(const TCHAR * filePath) const;

	/// Conditions

		bool IsConnected(int devID) const;
		bool ButtonPressed(int devID, int buttonID) const;
		bool ButtonHeldDown(int devID, int buttonID) const;
		bool ButtonReleased(int devID, int buttonID) const;
		bool AnyButtonPressed(int devID) const;
		bool AnyButtonHeldDown(int devID) const;
		bool AnyButtonReleased(int devID) const;

	/// Expressions

		int GetAxis(int joy, int axis) const;
		int GetButtonPressedState(int joy, int button) const;
		int GetHat(int joy, int hat) const;
		int GetBallXDelta(int joy, int ball) const;
		int GetBallYDelta(int joy, int ball) const;
		int NumAxes(int joy) const;
		int NumButtons(int joy) const;
		int NumHats(int joy) const;
		int NumBalls(int joy) const;
		int NumJoysticks() const;
		const TCHAR * GetDeviceGUID(int joy);
		const TCHAR * GetDeviceName(int joy);
		const TCHAR * GetButtonsHeldHex(int joy);
		int HeldButtonIndex(int joy, int index) const;
		int LastButtonPressedIndex(int joy) const;
		int LastButtonReleasedIndex(int joy) const;



	/* These are called if there's no function linked to an ID */

	void Action(int ID, RUNDATA * rdPtr, long param1, long param2);
	long Condition(int ID, RUNDATA * rdPtr, long param1, long param2);
	long Expression(int ID, RUNDATA * rdPtr, long param);




	/*  These replace the functions like HandleRunObject that used to be
		implemented in Runtime.cpp. They work exactly the same, but they're
		inside the extension class.
	*/

	REFLAG Handle();
	REFLAG Display();

	short Pause();
	short Continue();
};
