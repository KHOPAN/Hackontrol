#include "window_session_tabs.h"
#include <CommCtrl.h>

#define TAB_OFFSET 5

static void(__stdcall* sessionTabs[]) (const PTABINITIALIZER tab) = {
	WindowSessionTabStream,
	WindowSessionTabAudio
};

extern HINSTANCE instance;
extern HFONT font;

typedef struct {
	LPCWSTR name;
	TABCLIENTINITIALIZE clientInitialize;
	TABCLIENTUNINITIALIZE clientUninitialize;
	TABPACKETHANDLER packetHandler;
	BOOL alwaysProcessPacket;
	ULONGLONG data;
	LPCWSTR className;
	TABUNINITIALIZE uninitialize;
} TABDATA, *PTABDATA;

static PTABDATA tabData;

static void resizeTab(const PCLIENT client) {
	if(!client->session.selectedTab) {
		return;
	}

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

	client->session.selectedTab = client->session.tabs[index].tab;

	for(size_t i = 0; i < SIZEOFARRAY(sessionTabs); i++) {
		if(client->session.tabs[i].tab) {
			ShowWindow(client->session.tabs[i].tab, index == i ? SW_SHOW : SW_HIDE);
		}
	}

	resizeTab(client);
}

static LRESULT CALLBACK procedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
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
	tabData = KHOPAN_ALLOCATE(sizeof(TABDATA) * SIZEOFARRAY(sessionTabs));

	if(KHOPAN_ALLOCATE_FAILED(tabData)) {
		KHOPANERRORMESSAGE_WIN32(KHOPAN_ALLOCATE_ERROR, KHOPAN_ALLOCATE_FUNCTION);
		return FALSE;
	}

	size_t index;

	for(index = 0; index < sizeof(TABDATA) * SIZEOFARRAY(sessionTabs); index++) {
		((PBYTE) tabData)[index] = 0;
	}

	TABINITIALIZER initializer = {0};
	initializer.windowClass.cbSize = sizeof(WNDCLASSEXW);
	initializer.windowClass.lpfnWndProc = procedure;
	initializer.windowClass.hInstance = instance;
	initializer.windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	initializer.windowClass.hbrBackground = (HBRUSH) (COLOR_MENU + 1);
	initializer.windowClass.lpszClassName = CLASS_REMOTE_SESSION;

	if(!RegisterClassExW(&initializer.windowClass)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"RegisterClassExW");
		KHOPAN_DEALLOCATE(tabData);
		return FALSE;
	}

	size_t counter;

	for(index = 0; index < SIZEOFARRAY(sessionTabs); index++) {
		if(!sessionTabs[index]) {
			continue;
		}

		for(counter = 0; counter < sizeof(TABINITIALIZER); counter++) {
			((PBYTE) &initializer)[counter] = 0;
		}

		HBRUSH brush = CreateSolidBrush(0xF9F9F9);
		initializer.windowClass.cbSize = sizeof(WNDCLASSEXW);
		initializer.windowClass.hInstance = instance;
		initializer.windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
		initializer.windowClass.hbrBackground = brush;
		sessionTabs[index](&initializer);
		tabData[index].name = initializer.name;
		tabData[index].clientInitialize = initializer.clientInitialize;
		tabData[index].clientUninitialize = initializer.clientUninitialize;
		tabData[index].packetHandler = initializer.packetHandler;
		tabData[index].alwaysProcessPacket = initializer.alwaysProcessPacket;
		tabData[index].data = initializer.data;

		if(initializer.windowClass.lpszClassName) {
			if(!initializer.windowClass.lpfnWndProc) initializer.windowClass.lpfnWndProc = DefWindowProcW;
			counter = RegisterClassExW(&initializer.windowClass);
			if(counter) tabData[index].className = initializer.windowClass.lpszClassName;
			if(!counter) DeleteObject(brush);
		}

		if(initializer.initialize) {
			initializer.initialize(&tabData[index].data);
		}

		tabData[index].uninitialize = initializer.uninitialize;
	}

	return TRUE;
}

DWORD WINAPI WindowSession(_In_ PCLIENT client) {
	if(!client) {
		LOG("[Session]: Empty thread parameter\n");
		return 1;
	}

	client->session.tabs = KHOPAN_ALLOCATE(sizeof(TABSTORE) * SIZEOFARRAY(sessionTabs));
	DWORD codeExit = 1;
	size_t index;

	if(KHOPAN_ALLOCATE_FAILED(client->session.tabs)) {
		KHOPANERRORCONSOLE_WIN32(KHOPAN_ALLOCATE_ERROR, KHOPAN_ALLOCATE_FUNCTION);
		goto functionExit;
	}

	double screenWidth = GetSystemMetrics(SM_CXSCREEN);
	double screenHeight = GetSystemMetrics(SM_CYSCREEN);
	double width = screenWidth * 0.219619327;
	double height = screenHeight * 0.5859375;
	LPWSTR title = KHOPANFormatMessage(L"%ws [%ws]", client->address, client->name);
	client->session.window = CreateWindowExW(WS_EX_TOPMOST, CLASS_REMOTE_SESSION, title ? title : L"Session", WS_OVERLAPPEDWINDOW, (int) ((screenWidth - width) / 2.0), (int) ((screenHeight - height) / 2.0), (int) width, (int) height, NULL, NULL, instance, client);

	if(title) {
		LocalFree(title);
	}

	if(!client->session.window) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		goto freeTabs;
	}

	for(index = 0; index < SIZEOFARRAY(sessionTabs); index++) {
		if(tabData[index].clientInitialize) {
			client->session.tabs[index].data = 0;
			HWND window = tabData[index].clientInitialize(client, &client->session.tabs[index].data, client->session.window);
			client->session.tabs[index].tab = window ? window : NULL;
		}
	}

	client->session.tab = CreateWindowExW(0L, WC_TABCONTROL, L"", WS_CHILD | WS_TABSTOP | WS_VISIBLE, TAB_OFFSET, TAB_OFFSET, 0, 0, client->session.window, NULL, NULL, NULL);

	if(!client->session.tab) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");

		for(index = 0; index < SIZEOFARRAY(sessionTabs); index++) {
			if(tabData[index].clientUninitialize && client->session.tabs[index].tab) tabData[index].clientUninitialize(client, &client->session.tabs[index].data);
		}

		goto destroyWindow;
	}

	TCITEMW item = {0};
	item.mask = TCIF_TEXT;

	for(index = 0; index < SIZEOFARRAY(sessionTabs); index++) {
		item.pszText = (LPWSTR) tabData[index].name;
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

	for(index = 0; index < SIZEOFARRAY(sessionTabs); index++) {
		if(tabData[index].clientUninitialize && client->session.tabs[index].tab) {
			tabData[index].clientUninitialize(client, &client->session.tabs[index].data);
		}
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

BOOL WindowSessionHandlePacket(const PCLIENT client, const PHRSPPACKET packet) {
	for(size_t i = 0; i < SIZEOFARRAY(sessionTabs); i++) {
		if(!tabData[i].packetHandler) {
			continue;
		}

		PULONGLONG data = client->session.tabs ? &client->session.tabs[i].data : NULL;

		if(tabData[i].alwaysProcessPacket) {
			if(tabData[i].packetHandler(client, data, packet)) return TRUE;
			continue;
		}

		if(client->session.tabs && client->session.tabs[i].tab && tabData[i].packetHandler(client, data, packet)) {
			return TRUE;
		}
	}

	return FALSE;
}

void WindowSessionClose(const PCLIENT client) {
	if(client->session.window) {
		PostMessageW(client->session.window, WM_CLOSE, 0, 0);
	}
}

void WindowSessionCleanup() {
	for(size_t i = 0; i < SIZEOFARRAY(sessionTabs); i++) {
		if(tabData[i].className) {
			UnregisterClassW(tabData[i].className, instance);
		}

		if(tabData[i].uninitialize) {
			tabData[i].uninitialize(&tabData[i].data);
		}
	}

	UnregisterClassW(CLASS_REMOTE_SESSION, instance);
	KHOPAN_DEALLOCATE(tabData);
}
