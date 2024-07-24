#include <khopanwin32.h>
#include "server.h"

#define HACKONTROL_REMOTE L"HackontrolRemote"

static HWND globalWindow;
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

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE previousInstance, _In_ LPSTR argument, _In_ int commandLineShow) {
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

	globalWindow = CreateWindowExW(0, HACKONTROL_REMOTE, L"Hackontrol Remote", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, NULL, NULL, instance, NULL);
	int returnValue = 1;

	if(!globalWindow) {
		KHWin32DialogErrorW(GetLastError(), L"CreateWindowExW");
		goto unregisterWindowClass;
	}

	HANDLE serverThread = CreateThread(NULL, 0, ServerThread, NULL, 0, NULL);

	if(!serverThread) {
		HackontrolRemoteError(GetLastError(), L"CreateThread");
		goto unregisterWindowClass;
	}

	globalTitledBorder = CreateWindowExW(0, L"Button", L"Connected Devices", WS_CHILD | BS_GROUPBOX | WS_VISIBLE, 5, 0, 0, 0, globalWindow, NULL, NULL, NULL);

	if(!globalTitledBorder) {
		HackontrolRemoteError(GetLastError(), L"CreateWindowExW");
		goto closeServerThread;
	}

	NONCLIENTMETRICS metrics;
	metrics.cbSize = sizeof(NONCLIENTMETRICS);

	if(!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0)) {
		HackontrolRemoteError(GetLastError(), L"SystemParametersInfoW");
		goto closeServerThread;
	}

	HFONT font = CreateFontIndirectW(&metrics.lfCaptionFont);

	if(!font) {
		HackontrolRemoteError(GetLastError(), L"CreateFontIndirectW");
		goto closeServerThread;
	}

	SendMessageW(globalTitledBorder, WM_SETFONT, (WPARAM) font, TRUE);

	if(!SetWindowPos(globalWindow, HWND_TOP, 0, 0, 400, 400, SWP_NOMOVE)) {
		HackontrolRemoteError(GetLastError(), L"SetWindowPos");
		goto deleteFont;
	}

	ShowWindow(globalWindow, SW_NORMAL);
	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	returnValue = 0;
deleteFont:
	DeleteObject(font);
closeServerThread:
	CloseHandle(serverThread);
unregisterWindowClass:
	if(!UnregisterClassW(HACKONTROL_REMOTE, instance)) {
		KHWin32DialogErrorW(GetLastError(), L"UnregisterClassW");
		return 1;
	}

	return returnValue;
}

void HackontrolRemoteError(DWORD errorCode, const LPWSTR functionName) {
	LPWSTR message = KHWin32GetErrorMessageW(errorCode, functionName);
	MessageBoxW(globalWindow, message, L"Hackontrol Remote Error", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
	LocalFree(message);
	SendMessageW(globalWindow, WM_CLOSE, 0, 0);
}
