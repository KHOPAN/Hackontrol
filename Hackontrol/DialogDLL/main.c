#include <Windows.h>

__declspec(dllexport) void __stdcall Execute(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	MessageBoxW(NULL, L"!!OLD VERSION WARNING!!", L"Hackontrol", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
}
