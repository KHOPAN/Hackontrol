#include "update.h"

__declspec(dllexport) void __stdcall Update(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	DataStream stream = {0};

	if(!DownloadLatestLibdll32(&stream)) {
		return;
	}

	WaitForLibdll32ToExit((DWORD) atol(argument));

	if(!WriteLibdll32(&stream)) {
		return;
	}

	ExecuteLibdll32();
}
