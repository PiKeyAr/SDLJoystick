#include "Common.h"

void Extension::AddMappingsFromFile(const TCHAR * filePath) const
{
	TCHAR errorText[512], errorText2[256];
	FILE * file = NULL;

	errno_t error = _tfopen_s(&file, filePath, _T("rb"));
	if (error != NO_ERROR)
	{
		if (_tcserror_s(errorText2, error) != NO_ERROR)
			_stprintf_s(errorText2, _T("code %d"), error);

		_stprintf_s(errorText, _T("Failed to load controller mappings from \"%s\". Error %s."), filePath, errorText);
		MessageBox(NULL, errorText, _T(PROJECT_NAME " - Error loading mapping file"), MB_OK | MB_ICONERROR);
		return;
	}

	// The 1 at end of ...FromRW() auto-closes "file", including in error scenarios, so don't call fclose().

	// A community sourced database of controllers is available at https://raw.github.com/gabomdq/SDL_GameControllerDB/master/gamecontrollerdb.txt
	if (SDL_GameControllerAddMappingsFromRW(SDL_RWFromFP(file, SDL_TRUE), 1) == -1)
	{
		_stprintf_s(errorText, _T("Failed to read controller mappings from \"%s\". Error %hs."), filePath, SDL_GetError());
		MessageBox(NULL, errorText, _T(PROJECT_NAME " - Error reading mapping file"), MB_OK | MB_ICONERROR);
		return; 
	}
}
