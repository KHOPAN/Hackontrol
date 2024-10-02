#include <libkhopanlist.h>
#include "remote.h"
#include <CommCtrl.h>

#define IDM_REMOTE_OPEN          0xE001
#define IDM_REMOTE_DISCONNECT    0xE002
#define IDM_REMOTE_REFRESH       0xE003
#define IDM_REMOTE_ALWAYS_ON_TOP 0xE004
#define IDM_REMOTE_EXIT          0xE005

extern LINKEDLIST clientList;
extern HANDLE clientListMutex;

static HWND window;
static HWND border;
static HWND listView;

static LRESULT CALLBACK windowProcedure(_In_ HWND inputWindow, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	RECT bounds;
	LVHITTESTINFO information = {0};
	PLINKEDLISTITEM item;
	int status = 0;
	HMENU menu;
	BOOL topMost;

	switch(message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_SIZE:
		GetClientRect(window, &bounds);
		SetWindowPos(border, HWND_TOP, 0, 0, bounds.right - bounds.left - 10, bounds.bottom - bounds.top - 4, SWP_NOMOVE);
		GetClientRect(border, &bounds);
		SetWindowPos(listView, HWND_TOP, bounds.left + 9, bounds.top + 17, bounds.right - bounds.left - 8, bounds.bottom - bounds.top - 22, 0);
		return 0;
	case WM_CONTEXTMENU:
		GetCursorPos(&information.pt);
		ScreenToClient(listView, &information.pt);
		GetClientRect(window, &bounds);

		if(information.pt.x < bounds.left || information.pt.x > bounds.right || information.pt.y < bounds.top || information.pt.y > bounds.bottom) {
			return 0;
		}

		if(SendMessageW(listView, LVM_HITTEST, 0, (LPARAM) &information) != -1 && WaitForSingleObject(clientListMutex, INFINITE) != WAIT_FAILED) {
			status = KHOPANLinkedGet(&clientList, information.iItem, &item);
			ReleaseMutex(clientListMutex);
		}

		menu = CreatePopupMenu();

		if(!menu) {
			return 0;
		}

		if(status) {
			AppendMenuW(menu, MF_STRING, IDM_REMOTE_OPEN, L"Open");
			AppendMenuW(menu, MF_STRING, IDM_REMOTE_DISCONNECT, L"Disconnect");
			AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
		}

		topMost = GetWindowLongW(window, GWL_EXSTYLE) & WS_EX_TOPMOST;
		AppendMenuW(menu, MF_STRING, IDM_REMOTE_REFRESH, L"Refresh");
		AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
		AppendMenuW(menu, MF_STRING | (topMost ? MF_CHECKED : MF_UNCHECKED), IDM_REMOTE_ALWAYS_ON_TOP, L"Always On Top");
		AppendMenuW(menu, MF_STRING, IDM_REMOTE_EXIT, L"Exit");
		SetForegroundWindow(window);
		status = TrackPopupMenuEx(menu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, LOWORD(lparam), HIWORD(lparam), window, NULL);
		DestroyMenu(menu);

		switch(status) {
		case IDM_REMOTE_OPEN:
			return 1;
		case IDM_REMOTE_DISCONNECT:
			return 1;
		case IDM_REMOTE_REFRESH:
			return 1;
		case IDM_REMOTE_ALWAYS_ON_TOP:
			SetWindowPos(window, topMost ? HWND_NOTOPMOST : HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
			return 1;
		case IDM_REMOTE_EXIT:
			WindowMainExit();
			return 1;
		}

		return 0;
	}

	return DefWindowProcW(inputWindow, message, wparam, lparam);
}

int WindowMain(const HINSTANCE instance) {
	LOG("[Main Window]: Initializing\n");
	WNDCLASSEXW windowClass = {0};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.lpfnWndProc = windowProcedure;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH) COLOR_WINDOW;
	windowClass.lpszClassName = CLASS_HACKONTROL_REMOTE;

	if(!RegisterClassExW(&windowClass)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"RegisterClassExW");
		return 1;
	}

	INITCOMMONCONTROLSEX controls;
	controls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	controls.dwICC = ICC_LISTVIEW_CLASSES;
	int codeExit = 1;

	if(!InitCommonControlsEx(&controls)) {
		KHOPANERRORMESSAGE_WIN32(ERROR_FUNCTION_FAILED, L"InitCommonControlsEx");
		goto unregisterClass;
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int width = (int) (screenWidth * 0.292825769);
	int height = (int) (screenHeight * 0.78125);
	window = CreateWindowExW(WS_EX_TOPMOST, CLASS_HACKONTROL_REMOTE, L"Remote", WS_OVERLAPPEDWINDOW, (screenWidth - width) / 2, (screenHeight - height) / 2, width, height, NULL, NULL, instance, NULL);

	if(!window) {
		KHOPANLASTERRORMESSAGE_WIN32(L"CreateWindowExW");
		goto unregisterClass;
	}

	border = CreateWindowExW(WS_EX_NOPARENTNOTIFY, L"Button", L"Connected Devices", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 5, 0, 0, 0, window, NULL, NULL, NULL);

	if(!border) {
		KHOPANLASTERRORMESSAGE_WIN32(L"CreateWindowExW");
		goto unregisterClass;
	}

	listView = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | LVS_REPORT | LVS_SINGLESEL, 0, 0, 0, 0, window, NULL, NULL, NULL);

	if(!listView) {
		KHOPANLASTERRORMESSAGE_WIN32(L"CreateWindowExW");
		goto unregisterClass;
	}

	SendMessageW(listView, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	LVCOLUMNW column = {0};
	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	column.cx = (int) (screenWidth * 0.13250366);
	column.fmt = LVCFMT_LEFT;
	column.pszText = L"IP Address";

	if(SendMessageW(listView, LVM_INSERTCOLUMN, 0, (LPARAM) &column) == -1) {
		KHOPANLASTERRORMESSAGE_WIN32(L"ListView_InsertColumn");
		goto unregisterClass;
	}

	column.pszText = L"Username";

	if(SendMessageW(listView, LVM_INSERTCOLUMN, 0, (LPARAM) &column) == 1) {
		KHOPANLASTERRORMESSAGE_WIN32(L"ListView_InsertColumn");
		goto unregisterClass;
	}

	NONCLIENTMETRICS metrics;
	metrics.cbSize = sizeof(NONCLIENTMETRICS);

	if(!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"SystemParametersInfoW");
		goto unregisterClass;
	}

	HFONT font = CreateFontIndirectW(&metrics.lfCaptionFont);

	if(!font) {
		KHOPANLASTERRORMESSAGE_WIN32(L"CreateFontIndirectW");
		goto unregisterClass;
	}

	SendMessageW(border, WM_SETFONT, (WPARAM) font, TRUE);
	ShowWindow(window, SW_NORMAL);
	LOG("[Main Window]: Finished\n");
	CLIENT client;
	client.name = L"FakeKHOPAN";
	client.address = L"55.55.55.55";

	if(!KHOPANLinkedAdd(&clientList, (PBYTE) &client, NULL)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANLinkedAdd");
	}

	if(!KHOPANLinkedAdd(&clientList, (PBYTE) &client, NULL)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANLinkedAdd");
	}

	WindowMainRefresh();
	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	DeleteObject(font);
	codeExit = 0;
unregisterClass:
	UnregisterClassW(CLASS_HACKONTROL_REMOTE, instance);
	return codeExit;
}

void WindowMainRefresh() {
	if(WaitForSingleObject(clientListMutex, INFINITE) == WAIT_FAILED) {
		return;
	}

	SendMessageW(listView, LVM_DELETEALLITEMS, 0, 0);
	LVITEMW listItem = {0};
	listItem.mask = LVIF_TEXT;
	PLINKEDLISTITEM item;

	KHOPAN_LINKED_LIST_ITERATE(item, &clientList) {
		PCLIENT client = (PCLIENT) item->data;
		listItem.iSubItem = 0;
		listItem.pszText = client && client->name ? client->name : L"(Missing name)";
		SendMessageW(listView, LVM_INSERTITEM, 0, (LPARAM) &listItem);
		listItem.iSubItem = 1;
		listItem.pszText = client && client->address ? client->address : L"(Missing address)";
		SendMessageW(listView, LVM_SETITEM, 0, (LPARAM) &listItem);
	}

	ReleaseMutex(clientListMutex);
}

void WindowMainExit() {
	PostMessageW(window, WM_CLOSE, 0, 0);
}
