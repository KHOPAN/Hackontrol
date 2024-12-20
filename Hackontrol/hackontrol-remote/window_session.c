#include <hrsp_remote.h>
#include "window_session.h"
#include <CommCtrl.h>

static void(__stdcall* sessionTabs[]) (const PTABINITIALIZER tab) = {
	WindowSessionTabStream,
	WindowSessionTabAudio
};

#define TAB_OFFSET 5

#define CLASS_NAME L"HackontrolRemoteSession"

extern HINSTANCE instance;
extern HFONT font;

static struct TABDATA {
	LPCWSTR name;
	TABUNINITIALIZE uninitialize;
	TABCLIENTINITIALIZE clientInitialize;
	TABCLIENTUNINITIALIZE clientUninitialize;
	TABPACKETHANDLER packetHandler;
	BOOLEAN alwaysProcessPacket;
	ULONGLONG data;
	LPCWSTR className;
} *tabData;

static void resize(const PCLIENT client) {
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

	if(((LONGLONG) index) >= 0) {
		client->session.selectedTab = client->session.tabs[index].tab;

		for(size_t i = 0; i < sizeof(sessionTabs) / sizeof(sessionTabs[0]); i++) {
			if(client->session.tabs[i].tab) ShowWindow(client->session.tabs[i].tab, index == i ? SW_SHOW : SW_HIDE);
		}

		resize(client);
	}
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
	case WM_NOTIFY:
		if(!lparam) {
			break;
		}

		switch(((LPNMHDR) lparam)->code) {
		case TCN_SELCHANGE:
			selectTab(client);
		case TCN_SELCHANGING:
			return FALSE;
		}

		break;
	case WM_SIZE:
		GetClientRect(window, &bounds);
		SetWindowPos(client->session.tab, HWND_TOP, 0, 0, bounds.right - bounds.left - TAB_OFFSET * 2, bounds.bottom - bounds.top - TAB_OFFSET * 2, SWP_NOMOVE);
		resize(client);
		return 0;
	}

	return DefWindowProcW(window, message, wparam, lparam);
}

BOOLEAN WindowSessionInitialize() {
	tabData = KHOPAN_ALLOCATE(sizeof(sessionTabs) / sizeof(sessionTabs[0]) * sizeof(struct TABDATA));

	if(!tabData) {
		KHOPANERRORMESSAGE_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"KHOPAN_ALLOCATE");
		return FALSE;
	}

	TABINITIALIZER initializer = {0};
	initializer.windowClass.cbSize = sizeof(WNDCLASSEXW);
	initializer.windowClass.lpfnWndProc = procedure;
	initializer.windowClass.hInstance = instance;
	initializer.windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	initializer.windowClass.hbrBackground = (HBRUSH) (COLOR_MENU + 1);
	initializer.windowClass.lpszClassName = CLASS_NAME;

	if(!RegisterClassExW(&initializer.windowClass)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"RegisterClassExW");
		KHOPAN_DEALLOCATE(tabData);
		return FALSE;
	}

	for(size_t i = 0; i < sizeof(sessionTabs) / sizeof(sessionTabs[0]); i++) {
		size_t index;

		for(index = 0; index < sizeof(TABINITIALIZER); index++) {
			((PBYTE) &initializer)[index] = 0;
		}

		HBRUSH brush = CreateSolidBrush(0xF9F9F9);
		initializer.windowClass.cbSize = sizeof(WNDCLASSEXW);
		initializer.windowClass.hInstance = instance;
		initializer.windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
		initializer.windowClass.hbrBackground = brush;
		sessionTabs[i](&initializer);
		tabData[i].name = initializer.name;
		tabData[i].uninitialize = initializer.uninitialize;
		tabData[i].clientInitialize = initializer.clientInitialize;
		tabData[i].clientUninitialize = initializer.clientUninitialize;
		tabData[i].packetHandler = initializer.packetHandler;
		tabData[i].alwaysProcessPacket = initializer.alwaysProcessPacket;
		tabData[i].data = initializer.data;

		if(initializer.windowClass.lpszClassName) {
			if(!initializer.windowClass.lpfnWndProc) initializer.windowClass.lpfnWndProc = DefWindowProcW;
			index = RegisterClassExW(&initializer.windowClass);
			if(index) tabData[i].className = initializer.windowClass.lpszClassName;
			if(!index) DeleteObject(brush);
		}

		if(initializer.initialize) {
			initializer.initialize(&tabData[i].data);
		}
	}

	return TRUE;
}

DWORD WINAPI WindowSession(_In_ PCLIENT client) {
	if(!client) {
		LOG("[Session]: Empty thread parameter\n");
		return 1;
	}

	client->session.tabs = KHOPAN_ALLOCATE(sizeof(sessionTabs) / sizeof(sessionTabs[0]) * sizeof(TABSTORE));
	DWORD codeExit = 1;
	size_t i;

	if(!client->session.tabs) {
		KHOPANERRORCONSOLE_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"KHOPAN_ALLOCATE");
		goto functionExit;
	}

	double screenWidth = GetSystemMetrics(SM_CXSCREEN);
	double screenHeight = GetSystemMetrics(SM_CYSCREEN);
	double width = screenWidth * 0.219619327;
	double height = screenHeight * 0.5859375;
	LPWSTR title = KHOPANFormatMessage(L"%ws [%ws]", client->address, client->name);
	client->session.window = CreateWindowExW(WS_EX_TOPMOST, CLASS_NAME, title ? title : L"Session", WS_OVERLAPPEDWINDOW, (int) ((screenWidth - width) / 2.0), (int) ((screenHeight - height) / 2.0), (int) width, (int) height, NULL, NULL, instance, client);

	if(title) {
		KHOPAN_DEALLOCATE(title);
	}

	if(!client->session.window) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		goto freeTabs;
	}

	BYTE byte = 1;
	HRSPPACKET packet;
	packet.type = HRSP_REMOTE_SERVER_STATUS;
	packet.size = 1;
	packet.data = &byte;
	HRSPPacketSend(&client->hrsp, &packet, NULL);
	byte = 0;

	for(i = 0; i < sizeof(sessionTabs) / sizeof(sessionTabs[0]); i++) {
		if(tabData[i].clientInitialize) {
			client->session.tabs[i].data = 0;
			HWND window = tabData[i].clientInitialize(client, &client->session.tabs[i].data, client->session.window);
			client->session.tabs[i].tab = window ? window : NULL;
		}
	}

	client->session.tab = CreateWindowExW(WS_EX_COMPOSITED, WC_TABCONTROL, L"", WS_CHILD | WS_TABSTOP | WS_VISIBLE, TAB_OFFSET, TAB_OFFSET, 0, 0, client->session.window, NULL, NULL, NULL);

	if(!client->session.tab) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		HRSPPacketSend(&client->hrsp, &packet, NULL);
		DestroyWindow(client->session.window);
		goto sessionUninitialize;
	}

	TCITEMW item = {0};
	item.mask = TCIF_TEXT;

	for(i = 0; i < sizeof(sessionTabs) / sizeof(sessionTabs[0]); i++) {
		item.pszText = (LPWSTR) tabData[i].name;
		SendMessageW(client->session.tab, TCM_INSERTITEM, i, (LPARAM) &item);
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

	HRSPPacketSend(&client->hrsp, &packet, NULL);
	codeExit = 0;
sessionUninitialize:
	for(i = 0; i < sizeof(sessionTabs) / sizeof(sessionTabs[0]); i++) {
		if(tabData[i].clientUninitialize && client->session.tabs[i].tab) {
			tabData[i].clientUninitialize(client, &client->session.tabs[i].data);
		}
	}
freeTabs:
	KHOPAN_DEALLOCATE(client->session.tabs);
functionExit:
	if(codeExit != 0) {
		LOG("[Session %ws]: Exit with code: %d\n", client->address, codeExit);
	}

	CloseHandle(client->session.thread);

	for(i = 0; i < sizeof(SESSION); i++) {
		((PBYTE) &client->session)[i] = 0;
	}

	return codeExit;
}

BOOLEAN WindowSessionHandlePacket(const PCLIENT client, const PHRSPPACKET packet) {
	for(size_t i = 0; i < sizeof(sessionTabs) / sizeof(sessionTabs[0]); i++) {
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
	for(size_t i = 0; i < sizeof(sessionTabs) / sizeof(sessionTabs[0]); i++) {
		if(tabData[i].className) {
			UnregisterClassW(tabData[i].className, instance);
		}

		if(tabData[i].uninitialize) {
			tabData[i].uninitialize(&tabData[i].data);
		}
	}

	UnregisterClassW(CLASS_NAME, instance);
	KHOPAN_DEALLOCATE(tabData);
}
