#include "window_session_tabs.h"
#include <CommCtrl.h>

#define TAB_OFFSET 5

static SESSIONTAB sessionTabs[] = {
	{L"Stream", REMOTE_CLASS_TAB_STREAM, WindowSessionTabStream, WindowSessionTabStreamProcedure},
	{L"Audio",  NULL,                    NULL,                   NULL}
};

extern HINSTANCE instance;
extern HFONT font;

static void resizeTab(const PCLIENT client) {
	RECT bounds;
	GetClientRect(client->session.tab, &bounds);
	SendMessageW(client->session.tab, TCM_ADJUSTRECT, FALSE, (LPARAM) &bounds);
	SetWindowPos(client->session.selectedTab, HWND_TOP, bounds.left + TAB_OFFSET, bounds.top + TAB_OFFSET, bounds.right - bounds.left, bounds.bottom - bounds.top, 0);
}

static void selectTab(const PCLIENT client) {
	size_t index = SendMessageW(client->session.tab, TCM_GETCURSEL, 0, 0);

	if(((LONGLONG) index) == -1) {
		return;
	}

	client->session.selectedTab = client->session.tabs[index];

	for(size_t i = 0; i < SIZEOFARRAY(sessionTabs); i++) {
		ShowWindow(client->session.tabs[i], index == i ? SW_SHOW : SW_HIDE);
	}

	resizeTab(client);
}

static LRESULT CALLBACK windowProcedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	USERDATA(PCLIENT, client, window, message, wparam, lparam);
	RECT bounds;

	switch(message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_SIZE:
		GetClientRect(window, &bounds);
		SetWindowPos(client->session.tab, HWND_TOP, 0, 0, bounds.right - bounds.left - TAB_OFFSET * 2, bounds.bottom - bounds.top - TAB_OFFSET * 2, SWP_NOMOVE);
		resizeTab(client);
		return 0;
	case WM_NOTIFY:
		switch(((LPNMHDR) lparam)->code) {
		case TCN_SELCHANGING:
			return FALSE;
		case TCN_SELCHANGE:
			selectTab(client);
			return FALSE;
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
	windowClass.hbrBackground = (HBRUSH) (COLOR_MENU + 1);
	windowClass.lpszClassName = CLASS_REMOTE_SESSION;

	if(!RegisterClassExW(&windowClass)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"RegisterClassExW");
		return FALSE;
	}

	windowClass.hbrBackground = CreateSolidBrush(0xF9F9F9);

	for(size_t i = 0; i < SIZEOFARRAY(sessionTabs); i++) {
		if(sessionTabs[i].className) {
			windowClass.lpfnWndProc = sessionTabs[i].procedure ? sessionTabs[i].procedure : DefWindowProcW;
			windowClass.lpszClassName = sessionTabs[i].className;
			if(RegisterClassExW(&windowClass)) continue;
			KHOPANLASTERRORMESSAGE_WIN32(L"RegisterClassExW");
			return FALSE;
		}
	}

	return TRUE;
}

DWORD WINAPI WindowSession(_In_ PCLIENT client) {
	if(!client) {
		LOG("[Session]: Empty thread parameter\n");
		return 1;
	}

	client->session.tabs = KHOPAN_ALLOCATE(SIZEOFARRAY(sessionTabs) * sizeof(HWND));
	DWORD codeExit = 1;
	size_t index;

	if(KHOPAN_ALLOCATE_FAILED(client->session.tabs)) {
		KHOPANERRORCONSOLE_WIN32(KHOPAN_ALLOCATE_ERROR, KHOPAN_ALLOCATE_FUNCTION);
		goto functionExit;
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int width = (int) (screenWidth * 0.219619327);
	int height = (int) (screenHeight * 0.5859375);
	LPWSTR title = KHOPANFormatMessage(L"%ws [%ws]", client->name, client->address);
	client->session.window = CreateWindowExW(WS_EX_TOPMOST, CLASS_REMOTE_SESSION, title ? title : L"Session", WS_OVERLAPPEDWINDOW, (screenWidth - width) / 2, (screenHeight - height) / 2, width, height, NULL, NULL, instance, client);

	if(title) {
		LocalFree(title);
	}

	if(!client->session.window) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		goto freeTabs;
	}

	for(index = 0; index < SIZEOFARRAY(sessionTabs); index++) {
		if(sessionTabs[index].function) {
			HWND window = sessionTabs[index].function(client->session.window, client);
			client->session.tabs[index] = window ? window : NULL;
		}
	}

	client->session.tab = CreateWindowExW(0L, WC_TABCONTROL, L"", WS_CHILD | WS_VISIBLE, TAB_OFFSET, TAB_OFFSET, 0, 0, client->session.window, NULL, NULL, NULL);

	if(!client->session.tab) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		goto destroyWindow;
	}

	TCITEMW item = {0};
	item.mask = TCIF_TEXT;

	for(index = 0; index < SIZEOFARRAY(sessionTabs); index++) {
		item.pszText = sessionTabs[index].name;
		SendMessageW(client->session.tab, TCM_INSERTITEM, index, (LPARAM) &item);
	}

	SendMessageW(client->session.tab, WM_SETFONT, (WPARAM) font, TRUE);
	selectTab(client);
	ShowWindow(client->session.window, SW_NORMAL);
	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		if(!IsDialogMessageW(client->session.window, &message)) {
			TranslateMessage(&message);
			DispatchMessageW(&message);
		}
	}

	if(client->session.stream.thread) {
		WindowStreamClose(client);
		WaitForSingleObject(client->session.stream.thread, INFINITE);
	}

	codeExit = 0;
	goto freeTabs;
destroyWindow:
	DestroyWindow(client->session.window);
freeTabs:
	KHOPAN_DEALLOCATE(client->session.tabs);
functionExit:
	if(codeExit != 0) {
		LOG("[Session %ws]: Exit with code: %d\n", client->address, codeExit);
	}

	CloseHandle(client->session.thread);

	for(index = 0; index < sizeof(SESSION); index++) {
		((PBYTE) &client->session)[index] = 0;
	}

	return codeExit;
}

void WindowSessionClose(const PCLIENT client) {
	if(client->session.window) {
		PostMessageW(client->session.window, WM_CLOSE, 0, 0);
	}
}

void WindowSessionCleanup() {
	for(size_t i = 0; i < SIZEOFARRAY(sessionTabs); i++) {
		if(sessionTabs[i].className) {
			UnregisterClassW(sessionTabs[i].className, instance);
		}
	}

	UnregisterClassW(CLASS_REMOTE_SESSION, instance);
}
