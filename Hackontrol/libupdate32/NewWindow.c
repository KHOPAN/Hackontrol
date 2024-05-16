#include <Windows.h>

__declspec(dllexport) void __stdcall NewWindow(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	SetWindowPos(window, HWND_TOP, 100, 100, 400, 400, SWP_SHOWWINDOW);
	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}
}
