#include <khopanwin32.h>
#include "update.h"

__declspec(dllexport) void __stdcall Update(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	DWORD processIdentifier = (DWORD) atol(argument);
	WaitForLibdll32ToExit(processIdentifier);
	MessageBoxW(NULL, L"The process has exited", L"libupdate32", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
}
