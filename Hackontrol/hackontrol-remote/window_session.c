#include "remote.h"

extern HINSTANCE instance;

static LRESULT CALLBACK windowProcedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	PCLIENT client = (PCLIENT) GetWindowLongPtrW(window, GWLP_USERDATA);

	if(!client) {
		if(message != WM_CREATE) {
			return DefWindowProcW(window, message, wparam, lparam);
		}

		client = (PCLIENT) (((CREATESTRUCT*) lparam)->lpCreateParams);
		SetWindowLongPtrW(window, GWLP_USERDATA, (LONG_PTR) client);
	}

	switch(message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	/*case WM_COMMAND:
		if(HIWORD(wparam) != BN_CLICKED || WaitForSingleObject(client->session.mutex, INFINITE) == WAIT_FAILED) {
			break;
		}

		if(client->session.stream.thread) {
			WindowStreamClose(client);
			WaitForSingleObject(client->session.stream.thread, INFINITE);
			CloseHandle(client->session.stream.thread);
		}

		client->session.stream.thread = CreateThread(NULL, 0, WindowStream, client, 0, NULL);

		if(!client->session.stream.thread) {
			KHOPANLASTERRORCONSOLE_WIN32(L"CreateThread");
		}

		ReleaseMutex(client->session.mutex);
		break;*/
	}

	return DefWindowProcW(window, message, wparam, lparam);
}

BOOL WindowSessionInitialize() {
	WNDCLASSEXW windowClass = {0};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.lpfnWndProc = windowProcedure;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH) COLOR_WINDOW;
	windowClass.lpszClassName = CLASS_REMOTE_SESSION;

	if(!RegisterClassExW(&windowClass)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"RegisterClassExW");
		return FALSE;
	}

	return TRUE;
}

DWORD WINAPI WindowSession(_In_ PCLIENT client) {
	if(!client) {
		LOG("[Session]: Empty thread parameter\n");
		return 1;
	}

	LOG("[Session %ws]: Initializing\n", client->address);
	RECT bounds;
	bounds.right = GetSystemMetrics(SM_CXSCREEN);
	bounds.bottom = GetSystemMetrics(SM_CYSCREEN);
	bounds.left = (long) (((double) bounds.right) * 0.146412884);
	bounds.top = (long) (((double) bounds.bottom) * 0.130208333);
	LPWSTR title = KHOPANFormatMessage(L"%ws [%ws]", client->name, client->address);
	client->session.window = CreateWindowExW(WS_EX_TOPMOST, CLASS_REMOTE_SESSION, title ? title : L"Session", WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU | WS_VISIBLE, (bounds.right - bounds.left) / 2, (bounds.bottom - bounds.top) / 2, bounds.left, bounds.top, NULL, NULL, instance, client);

	if(title) {
		LocalFree(title);
	}

	DWORD codeExit = 1;

	if(!client->session.window) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		goto functionExit;
	}

	int buttonWidth = bounds.top;
	int buttonHeight = buttonWidth / 4;
	GetClientRect(client->session.window, &bounds);
	bounds.right -= bounds.left;
	bounds.bottom -= bounds.top;
	HWND button = CreateWindowExW(0L, L"Button", L"Stream", WS_TABSTOP | WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, (bounds.right - buttonWidth) / 2, (bounds.bottom - buttonHeight) / 2, buttonWidth, buttonHeight, client->session.window, NULL, NULL, NULL);

	if(!button) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		goto destroyWindow;
	}

	NONCLIENTMETRICS metrics;
	metrics.cbSize = sizeof(NONCLIENTMETRICS);

	if(!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0)) {
		KHOPANLASTERRORCONSOLE_WIN32(L"SystemParametersInfoW");
		goto destroyButton;
	}

	HFONT font = CreateFontIndirectW(&metrics.lfCaptionFont);

	if(!font) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateFontIndirectW");
		goto destroyButton;
	}

	SendMessageW(button, WM_SETFONT, (WPARAM) font, TRUE);
	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	DeleteObject(font);

	if(client->session.stream.thread) {
		WindowStreamClose(client);
		WaitForSingleObject(client->session.stream.thread, INFINITE);
		CloseHandle(client->session.stream.thread);
	}

	codeExit = 0;
destroyButton:
	DestroyWindow(button);
destroyWindow:
	DestroyWindow(client->session.window);
	client->session.window = NULL;
functionExit:
	LOG("[Session %ws]: Exit with code: %d\n", client->address, codeExit);
	return codeExit;
}

void WindowSessionClose(const PCLIENT client) {
	if(client->session.window) {
		PostMessageW(client->session.window, WM_CLOSE, 0, 0);
	}
}
