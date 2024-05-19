#include <khopanwin32.h>
#include "update.h"

__declspec(dllexport) void __stdcall Update(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	WaitForLibdll32ToExit((DWORD) atol(argument));
	DownloadLatestLibdll32();
	MessageBoxA(NULL, "Done", "libupdate32", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
}
