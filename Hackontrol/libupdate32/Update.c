#include "update.h"

__declspec(dllexport) void __stdcall Update(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	WaitForLibdll32ToExit((DWORD) atol(argument));
	DownloadLatestLibdll32();
	ExecuteLibdll32();
}
