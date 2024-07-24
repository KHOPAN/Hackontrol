#include <khopanwin32.h>

#define HACKONTROL_REMOTE L"HackontrolRemote"

static HWND globalTitledBorder;

static LRESULT CALLBACK hackontrolRemoteProcedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	switch(message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_SIZE:
		SetWindowPos(globalTitledBorder, HWND_TOP, 0, 0, LOWORD(lparam) - 10, HIWORD(lparam) - 5, SWP_NOMOVE);
		return 0;
	}

	return DefWindowProcW(window, message, wparam, lparam);
}

int window(HINSTANCE instance) {
	WNDCLASSEXW windowClass = {0};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.lpfnWndProc = hackontrolRemoteProcedure;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH) COLOR_WINDOW;
	windowClass.lpszClassName = HACKONTROL_REMOTE;

	if(!RegisterClassExW(&windowClass)) {
		KHWin32DialogErrorW(GetLastError(), L"RegisterClassExW");
		return 1;
	}

	HWND window = CreateWindowExW(0, HACKONTROL_REMOTE, L"Hackontrol Remote", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, NULL, NULL, instance, NULL);
	int returnValue = 1;

	if(!window) {
		KHWin32DialogErrorW(GetLastError(), L"CreateWindowExW");
		goto unregisterWindowClass;
	}

	globalTitledBorder = CreateWindowExW(0, L"Button", L"Connected Devices", WS_CHILD | BS_GROUPBOX | WS_VISIBLE, 5, 0, 0, 0, window, NULL, NULL, NULL);

	if(!globalTitledBorder) {
		KHWin32DialogErrorW(GetLastError(), L"CreateWindowExW");
		goto unregisterWindowClass;
	}

	NONCLIENTMETRICS metrics;
	metrics.cbSize = sizeof(NONCLIENTMETRICS);

	if(!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0)) {
		KHWin32DialogErrorW(GetLastError(), L"SystemParametersInfoW");
		goto unregisterWindowClass;
	}

	HFONT font = CreateFontIndirectW(&metrics.lfCaptionFont);

	if(!font) {
		KHWin32DialogErrorW(GetLastError(), L"CreateFontIndirectW");
		goto unregisterWindowClass;
	}

	SendMessageW(globalTitledBorder, WM_SETFONT, (WPARAM) font, TRUE);

	if(!SetWindowPos(window, HWND_TOP, 0, 0, 400, 400, SWP_NOMOVE)) {
		KHWin32DialogErrorW(GetLastError(), L"SetWindowPos");
		goto unregisterWindowClass;
	}

	ShowWindow(window, SW_NORMAL);
	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	DeleteObject(font);
	returnValue = 0;
unregisterWindowClass:
	if(!UnregisterClassW(HACKONTROL_REMOTE, instance)) {
		KHWin32DialogErrorW(GetLastError(), L"UnregisterClassW");
		return 1;
	}

	return 0;
}
