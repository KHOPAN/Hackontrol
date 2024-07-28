#include <khopanwin32.h>
#include "window_main.h"

static HINSTANCE windowInstance;
static HWND window;

BOOL InitializeMainWindow(const HINSTANCE instance) {
	windowInstance = instance;
	WNDCLASSEXW windowClass = {0};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.lpfnWndProc = DefWindowProcW;
	windowClass.hInstance = windowInstance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH) COLOR_WINDOW;
	windowClass.lpszClassName = CLASS_HACKONTROL_REMOTE;

	if(!RegisterClassExW(&windowClass)) {
		KHWin32DialogErrorW(GetLastError(), L"RegisterClassExW");
		return FALSE;
	}

	window = CreateWindowExW(0, CLASS_HACKONTROL_REMOTE, L"Hackontrol Remote", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, NULL, NULL, windowInstance, NULL);

	if(!window) {
		KHWin32DialogErrorW(GetLastError(), L"CreateWindowExW");
		return FALSE;
	}

	return TRUE;
}

BOOL MainWindowMessageLoop() {
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int width = (int) (((double) screenWidth) * 0.292825769);
	int height = (int) (((double) screenHeight) * 0.78125);
	BOOL returnValue = FALSE;

	if(!SetWindowPos(window, HWND_TOPMOST, (screenWidth - width) / 2, (screenHeight - height) / 2, width, height, SWP_SHOWWINDOW)) {
		KHWin32DialogErrorW(GetLastError(), L"SetWindowPos");
		return FALSE;
	}

	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	return TRUE;
}
