#include <khopanwin32.h>
#include "update.h"

__declspec(dllexport) void __stdcall Update(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	BYTE* data = DownloadLatestLibdll32();

	if(!data) {
		return;
	}
	
	WaitForLibdll32ToExit((DWORD) atol(argument));
	MessageBoxA(NULL, (LPSTR) data, "libupdate32", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
}
