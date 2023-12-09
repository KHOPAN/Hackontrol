#include <Windows.h>
#include "HackontrolUpdater.h"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR argument, int command) {
	CURL* curl = HU_InitializeCURL();
	const char* versionFile = HU_GetVersionFile(curl);
	MessageBoxA(NULL, versionFile, NULL, MB_OK);
	return 0;
}
