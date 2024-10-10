#include "remote.h"

extern HINSTANCE instance;

static LRESULT CALLBACK windowProcedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	switch(message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProcW(window, message, wparam, lparam);
}

BOOL WindowStreamInitialize() {
	WNDCLASSEXW windowClass = {0};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.lpfnWndProc = windowProcedure;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH) COLOR_WINDOW;
	windowClass.lpszClassName = CLASS_SESSION_STREAM;

	if(!RegisterClassExW(&windowClass)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"RegisterClassExW");
		return FALSE;
	}

	return TRUE;
}

DWORD WINAPI WindowStream(_In_ PCLIENT client) {
	if(!client) {
		LOG("[Stream]: Empty thread parameter\n");
		return 1;
	}

	LOG("[Stream %ws]: Initializing\n", client->address);
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int width = (int) (((double) screenWidth) * 0.439238653);
	int height = (int) (((double) screenHeight) * 0.520833333);
	LPWSTR title = KHOPANFormatMessage(L"Livestream [%ws]", client->name);
	client->session.stream.window = CreateWindowExW(WS_EX_TOPMOST, CLASS_SESSION_STREAM, title ? title : L"Livestream", WS_OVERLAPPEDWINDOW | WS_VISIBLE, (screenWidth - width) / 2, (screenHeight - height) / 2, width, height, NULL, NULL, instance, client);

	if(title) {
		LocalFree(title);
	}

	DWORD codeExit = 1;

	if(!client->session.stream.window) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		goto functionExit;
	}

	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	codeExit = 0;
	DestroyWindow(client->session.stream.window);
	client->session.stream.window = NULL;
functionExit:
	LOG("[Stream %ws]: Exit with code: %d\n", client->address, codeExit);
	CloseHandle(client->session.stream.thread);
	client->session.stream.thread = NULL;
	return codeExit;
}

void WindowStreamClose(const PCLIENT client) {
	if(client->session.stream.window) {
		PostMessageW(client->session.stream.window, WM_CLOSE, 0, 0);
	}
}
