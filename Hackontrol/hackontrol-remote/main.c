#include <khopanwin32.h>
#include <CommCtrl.h>
#include "server.h"

#define HACKONTROL_REMOTE L"HackontrolRemote"

static int globalExitCode;
static HWND globalWindow;
static HWND globalTitledBorder;
static HWND globalListView;

static LRESULT CALLBACK hackontrolRemoteProcedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	switch(message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(globalExitCode);
		return 0;
	case WM_SIZE: {
		RECT bounds;
		GetClientRect(window, &bounds);
		SetWindowPos(globalTitledBorder, HWND_TOP, 0, 0, bounds.right - bounds.left - 10, bounds.bottom - bounds.top - 4, SWP_NOMOVE);
		GetClientRect(globalTitledBorder, &bounds);
		SetWindowPos(globalListView, HWND_TOP, bounds.left + 9, bounds.top + 15, bounds.right - bounds.left - 8, bounds.bottom - bounds.top - 20, 0);
		return 0;
	}
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

	globalTitledBorder = CreateWindowExW(0, L"Button", L"Connected Devices", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 5, 0, 0, 0, globalWindow, NULL, NULL, NULL);

	if(!globalTitledBorder) {
		HackontrolRemoteError(GetLastError(), L"CreateWindowExW");
		goto closeServerThread;
	}

	INITCOMMONCONTROLSEX controls;
	controls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	controls.dwICC = ICC_LISTVIEW_CLASSES;

	if(!InitCommonControlsEx(&controls)) {
		HackontrolRemoteError(ERROR_FUNCTION_FAILED, L"InitCommonControlsEx");
		goto closeServerThread;
	}

	globalListView = CreateWindowExW(WS_EX_NOPARENTNOTIFY | WS_EX_CLIENTEDGE | LVS_EX_FULLROWSELECT, WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL, 0, 0, 0, 0, globalWindow, NULL, NULL, NULL);

	if(!globalListView) {
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
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int width = (int) (((double) screenWidth) * 0.292825769);
	int height = (int) (((double) screenHeight) * 0.78125);

	if(!SetWindowPos(globalWindow, HWND_TOP, (screenWidth - width) / 2, (screenHeight - height) / 2, width, height, SWP_SHOWWINDOW)) {
		HackontrolRemoteError(GetLastError(), L"SetWindowPos");
		goto deleteFont;
	}

	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	returnValue |= globalExitCode;
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

void ExitHackontrolRemote(int exitCode) {
	globalExitCode = exitCode;
	SendMessageW(globalWindow, WM_CLOSE, 0, 0);
}

void HackontrolRemoteError(DWORD errorCode, const LPWSTR functionName) {
	LPWSTR message = KHWin32GetErrorMessageW(errorCode, functionName);
	MessageBoxW(globalWindow, message, L"Hackontrol Remote Error", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
	LocalFree(message);
	ExitHackontrolRemote(errorCode);
}
