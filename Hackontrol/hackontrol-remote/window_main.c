#include "remote.h"

static HWND window;

int WindowMain(const HINSTANCE instance) {
	LOG("[Main Window]: Initializing\n");
	WNDCLASSEXW windowClass = {0};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.lpfnWndProc = DefWindowProcW;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH) COLOR_WINDOW;
	windowClass.lpszClassName = CLASS_HACKONTROL_REMOTE;

	if(!RegisterClassExW(&windowClass)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"RegisterClassExW");
		return 1;
	}

	int codeExit = 1;
	window = CreateWindowExW(WS_EX_TOPMOST, CLASS_HACKONTROL_REMOTE, L"Remote", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 0, 0, NULL, NULL, instance, NULL);

	if(!window) {
		KHOPANLASTERRORMESSAGE_WIN32(L"CreateWindowExW");
		goto functionExit;
	}

	LOG("[Main Window]: Finished\n");
	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	if(!UnregisterClassW(CLASS_HACKONTROL_REMOTE, instance)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"UnregisterClassW");
		goto functionExit;
	}
functionExit:
	return codeExit;
}

void WindowMainExit() {
	PostMessageW(window, WM_CLOSE, 0, 0);
}
