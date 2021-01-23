# SDL Joystick Object by PkR

SDL Joystick is an extension for Clickteam Fusion 2.5 that makes use of the [SDL2 Joystick API](https://www.libsdl.org/release/SDL-1.2.15/docs/html/joystick.html) to interface with game controllers.

This is a DarkEdif port by Phi that does not require Visual Studio Runtime Redistributable, or SDL.dll.

This build instead makes use of a static build of SDL, limited to just including Joystick-related and dependents from SDL2.

SDL Joystick uses the SDL2 library, and you must conform to its [license terms](https://www.libsdl.org/license.php) in all projects using this extension.

### To update SDL
The basic process to update SDL is:
1. Download source from [here](https://www.libsdl.org/download-2.0.php), extract it.  
  **WARNING:** Do not have spaces in your any folder of your source or build path. CMake will not work with it and cause compiling issues later. You cannot move the source folder without deleting CMake cache and redoing Steps 3-6.
2. Run CMake GUI on the folder with source, then press Configure.  
  Don't forget to set a build location: I set mine to just `source folder\build`.
3. It will ask you what platform and toolset to use  
  Under platform, pick `Win32`  
  Under toolset, put `v141_xp` for XP+ compatibility, otherwise leave blank for Vista+.  
  ![options](https://cdn.discordapp.com/attachments/409878770242289666/773008157361307648/unknown.png)  
4. Press OK, it'll load the list of values that you see above.
* Check the Grouped and Advanced checkboxes above the list.
* Under CMAKE, change any "/MDd" to "/MTd", which will make SDL not depend on VS redistributable; likewise, all "/MD" to "/MT". This switches Runtime Library.
  For XP compatibility, under CMAKE_C_FLAGS and CMAKE_CXX_FLAGS, add a `/DWINVER=0x0501 /D_WIN32_WINNT=0x0501`.  
  ![define example](https://cdn.discordapp.com/attachments/409878770242289666/773008280145625138/unknown.png)
* Under RENDER category, disable all.
* Under SDL category, enable SDL_EVENTS, HAPTIC, LOADSO, JOYSTICK, STATIC, VIDEO. Disable the rest.
* Under VIDEO category, enable VIDEO_DUMMY. Disable the rest.
* Under Ungrouped, enable LIBC. **Do not change the rest.**
5. Press Configure again. If no error, then press Generate.
6. Press Open Project button.
7. In the project, find "SDL_dynapi.h", and set `#define SDL_DYNAMIC_API` to 0 at the bottom of the file.  
  (This file will have a lot of warnings about not doing a static build due to difficulty upgrading, but they don't apply to Fusion users, only to more lowlevel developers.)
8. In the project, go to "SDLsource\build\include\SDL_config.h", and where `#define HAS_WCSLCPY 1` and `#define HAS_WCSLCAT 1` are, **comment them out**.  
  (These are mistakenly detected and will cause linker issues)
9. You're done; press to build Debug and Release configurations.  
  (If they do not build and say files are missing, you didn't read the warning in step 1.)


The resulting files:
* Copy include files from SDLsource\include -> repo\DarkEdif\Inc\Shared\SDL\
* Replace SDLsource\build\include\SDL_config.h -> repo\DarkEdif\Inc\Shared\SDL\SDL_config.h
* Copy lib files from SDLsource\build\Debug -> repo\DarkEdif\Lib\Windows
* Copy lib files from SDLsource\build\Release -> repo\DarkEdif\Lib\Windows
10. All done. Next build of the extension will have the latest SDL2.

Note that if you're porting a different extension, Extension.h tags multiple more OS libraries in `#pragma comment (lib, "x.lib")` that SDL desires.
