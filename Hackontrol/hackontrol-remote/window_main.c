#include <khopanwin32.h>
#include "window_main.h"

static HWND window;

BOOL InitializeMainWindow(const HINSTANCE instance) {
	WNDCLASSEXW windowClass = {0};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.lpfnWndProc = DefWindowProcW;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH) COLOR_WINDOW;
	windowClass.lpszClassName = CLASS_HACKONTROL_REMOTE;

	if(!RegisterClassExW(&windowClass)) {
		KHWin32DialogErrorW(GetLastError(), L"RegisterClassExW");
		return FALSE;
	}

	window = CreateWindowExW(WS_EX_TOPMOST, CLASS_HACKONTROL_REMOTE, L"Hackontrol Remote", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, NULL, NULL, instance, NULL);

	if(!window) {
		KHWin32DialogErrorW(GetLastError(), L"CreateWindowExW");
		goto unregisterClass;
	}
unregisterClass:
	UnregisterClassW(CLASS_HACKONTROL_REMOTE, instance);
	return TRUE;
}
