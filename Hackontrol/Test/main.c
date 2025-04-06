#include <Windows.h>
#include "header.h"

#define CLASS_NAME L"WindowClass"

static int x;

static LRESULT CALLBACK procedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	switch(message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	//case WM_PAINT:
	//	drawDirect2D(x);
	//	return 0;
	}

	return DefWindowProcW(window, message, wparam, lparam);
}

static void CALLBACK timerFunction(HWND window, UINT message, UINT_PTR identifier, DWORD whatIsThis) {
	x++;
	drawDirect2D(x);
	//InvalidateRect(window, NULL, TRUE);
}

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE previousInstance, _In_ LPSTR command, _In_ int show) {
	WNDCLASSW windowClass = {0};
	windowClass.lpfnWndProc = procedure;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	windowClass.lpszClassName = CLASS_NAME;

	if(!RegisterClassW(&windowClass)) {
		return 1;
	}

	HWND window = CreateWindowExW(0L, CLASS_NAME, L"Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 600, 400, NULL, NULL, instance, NULL);
	int codeExit = 1;

	if(!window) {
		goto unregisterClass;
	}

	SetTimer(window, 1, 10, timerFunction);

	if(!initializeDirect2D(window)) {
		goto unregisterClass;
	}

	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	codeExit = 0;
unregisterClass:
	UnregisterClassW(CLASS_NAME, instance);
	return codeExit;
}
