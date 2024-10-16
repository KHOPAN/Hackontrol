#include "remote.h"
#include <CommCtrl.h>

static SESSIONTAB sessionTabs[] = {
	{L"Stream"},
	{L"Audio"}
};

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
	case WM_COMMAND:
		if(HIWORD(wparam) != BN_CLICKED) {
			break;
		}

		if(client->session.stream.mutex) {
			WaitForSingleObject(client->session.stream.mutex, INFINITE);
			ReleaseMutex(client->session.stream.mutex);
		}

		if(client->session.stream.thread) {
			WindowStreamClose(client);
			WaitForSingleObject(client->session.stream.thread, INFINITE);
		}

		client->session.stream.thread = CreateThread(NULL, 0, WindowStream, client, 0, NULL);

		if(!client->session.stream.thread) {
			KHOPANLASTERRORCONSOLE_WIN32(L"CreateThread");
		}

		break;
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

	RECT bounds;
	bounds.right = GetSystemMetrics(SM_CXSCREEN);
	bounds.bottom = GetSystemMetrics(SM_CYSCREEN);
	bounds.left = (long) (((double) bounds.right) * 0.146412884);
	bounds.top = (long) (((double) bounds.bottom) * 0.130208333);
	LPWSTR title = KHOPANFormatMessage(L"%ws [%ws]", client->name, client->address);
	client->session.window = CreateWindowExW(WS_EX_TOPMOST, CLASS_REMOTE_SESSION, title ? title : L"Session", WS_OVERLAPPEDWINDOW | WS_VISIBLE, (bounds.right - bounds.left) / 2, (bounds.bottom - bounds.top) / 2, bounds.left, bounds.top, NULL, NULL, instance, client);

	if(title) {
		LocalFree(title);
	}

	DWORD codeExit = 1;

	if(!client->session.window) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		goto functionExit;
	}

	INITCOMMONCONTROLSEX controls;
	controls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	controls.dwICC = ICC_TAB_CLASSES;

	if(!InitCommonControlsEx(&controls)) {
		KHOPANERRORCONSOLE_WIN32(ERROR_FUNCTION_FAILED, L"InitCommonControlsEx");
		goto destroyWindow;
	}

	HWND tab = CreateWindowExW(0L, WC_TABCONTROL, L"", WS_CHILD | WS_VISIBLE, 10, 10, 200, 200, client->session.window, NULL, NULL, NULL);

	if(!tab) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		goto destroyWindow;
	}

	TCITEMW item = {0};
	item.mask = TCIF_TEXT;

	for(size_t i = 0; i < sizeof(sessionTabs) / sizeof(sessionTabs[0]); i++) {
		item.pszText = sessionTabs[i].name;
		SendMessageW(tab, TCM_INSERTITEM, i, (LPARAM) &item);
	}

	NONCLIENTMETRICS metrics;
	metrics.cbSize = sizeof(NONCLIENTMETRICS);

	if(!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0)) {
		KHOPANLASTERRORCONSOLE_WIN32(L"SystemParametersInfoW");
		goto destroyWindow;
	}

	HFONT font = CreateFontIndirectW(&metrics.lfCaptionFont);

	if(!font) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateFontIndirectW");
		goto destroyWindow;
	}

	SendMessageW(tab, WM_SETFONT, (WPARAM) font, TRUE);
	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	DeleteObject(font);

	if(client->session.stream.thread) {
		WindowStreamClose(client);
		WaitForSingleObject(client->session.stream.thread, INFINITE);
	}

	codeExit = 0;
	goto functionExit;
destroyWindow:
	DestroyWindow(client->session.window);
functionExit:
	if(codeExit != 0) {
		LOG("[Session %ws]: Exit with code: %d\n", client->address, codeExit);
	}

	CloseHandle(client->session.thread);
	client->session.window = NULL;
	client->session.thread = NULL;
	return codeExit;
}

void WindowSessionClose(const PCLIENT client) {
	if(client->session.window) {
		PostMessageW(client->session.window, WM_CLOSE, 0, 0);
	}
}
