#include "thread_client.h"
#include <CommCtrl.h>
#include <khopanwin32.h>
#include <khopanarray.h>
#include "logger.h"
#include "window_main.h"

#define CLASS_HACKONTROL_REMOTE L"HackontrolRemote"

#define IDM_REMOTE_OPEN          0xE001
#define IDM_REMOTE_DISCONNECT    0xE002
#define IDM_REMOTE_REFRESH       0xE003
#define IDM_REMOTE_ALWAYS_ON_TOP 0xE004
#define IDM_REMOTE_EXIT          0xE005

#pragma warning(disable: 26454)

extern ArrayList clients;
extern HANDLE clientsLock;

static HWND window;
static HWND titledBorder;
static HWND listView;

static BOOL activeItem(size_t index, PCLIENT* client) {
	if(WaitForSingleObject(clientsLock, INFINITE) == WAIT_FAILED) {
		return FALSE;
	}

	size_t pointer = 0;
	PCLIENT instance;

	for(size_t i = 0; i < clients.elementCount; i++) {
		if(!KHArrayGet(&clients, i, &instance)) {
			ReleaseMutex(clientsLock);
			return FALSE;
		}

		if(!instance->active) {
			continue;
		}

		if(index == pointer) {
			(*client) = instance;
			ReleaseMutex(clientsLock);
			return TRUE;
		}

		pointer++;
	}

	ReleaseMutex(clientsLock);
	return FALSE;
}

static LRESULT CALLBACK windowProcedure(_In_ HWND inputWindow, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	PCLIENT client = NULL;

	switch(message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_NOTIFY: {
		if(((LPNMHDR) lparam)->code != NM_DBLCLK) {
			return 0;
		}

		if(activeItem(((LPNMITEMACTIVATE) lparam)->iItem, &client)) {
			ClientOpen(client);
		}

		return DefWindowProcW(inputWindow, message, wparam, lparam);
	}
	}

	RECT bounds;

	switch(message) {
	case WM_SIZE:
		GetClientRect(window, &bounds);
		SetWindowPos(titledBorder, HWND_TOP, 0, 0, bounds.right - bounds.left - 10, bounds.bottom - bounds.top - 4, SWP_NOMOVE);
		GetClientRect(titledBorder, &bounds);
		SetWindowPos(listView, HWND_TOP, bounds.left + 9, bounds.top + 17, bounds.right - bounds.left - 8, bounds.bottom - bounds.top - 22, 0);
		return 0;
	case WM_CONTEXTMENU: {
		LVHITTESTINFO hitTest = {0};
		hitTest.pt.x = LOWORD(lparam);
		hitTest.pt.y = HIWORD(lparam);
		ScreenToClient(listView, &hitTest.pt);
		GetClientRect(listView, &bounds);

		if(hitTest.pt.x < bounds.left || hitTest.pt.x > bounds.right || hitTest.pt.y < bounds.top || hitTest.pt.y > bounds.bottom) {
			return 0;
		}

		HMENU popupMenu = CreatePopupMenu();

		if(!popupMenu) {
			return 0;
		}

		if(SendMessageW(listView, LVM_HITTEST, 0, (LPARAM) &hitTest) != -1 && activeItem(hitTest.iItem, &client)) {
			AppendMenuW(popupMenu, MF_STRING, IDM_REMOTE_OPEN, L"Open");
			AppendMenuW(popupMenu, MF_STRING, IDM_REMOTE_DISCONNECT, L"Disconnect");
			AppendMenuW(popupMenu, MF_SEPARATOR, 0, NULL);
		}

		AppendMenuW(popupMenu, MF_STRING, IDM_REMOTE_REFRESH, L"Refresh");
		AppendMenuW(popupMenu, MF_SEPARATOR, 0, NULL);
		AppendMenuW(popupMenu, MF_STRING | (GetWindowLongW(window, GWL_EXSTYLE) & WS_EX_TOPMOST ? MF_CHECKED : MF_UNCHECKED), IDM_REMOTE_ALWAYS_ON_TOP, L"Always On Top");
		AppendMenuW(popupMenu, MF_STRING, IDM_REMOTE_EXIT, L"Exit");
		SetForegroundWindow(window);
		BOOL response = TrackPopupMenuEx(popupMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, LOWORD(lparam), HIWORD(lparam), window, NULL);
		DestroyMenu(popupMenu);

		switch(response) {
		case IDM_REMOTE_OPEN:
			ClientOpen(client);
			break;
		case IDM_REMOTE_DISCONNECT:
			LOG("[Window]: Disconnecting % ws\n" COMMA client ? client->address : L"(Missing address)");
			ClientDisconnect(client);
			break;
		case IDM_REMOTE_REFRESH:
			LOG("[Window]: Refreshing list view\n");
			MainWindowRefreshListView();
			break;
		case IDM_REMOTE_ALWAYS_ON_TOP:
			SetWindowPos(window, (GetWindowLongW(window, GWL_EXSTYLE) & WS_EX_TOPMOST) ? HWND_NOTOPMOST : HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
			break;
		case IDM_REMOTE_EXIT:
			LOG("[Window]: Exiting\n");
			MainWindowExit();
			break;
		}

		return 0;
	}
	}

	return DefWindowProcW(inputWindow, message, wparam, lparam);
}

BOOL MainWindowInitialize(const HINSTANCE instance) {
	WNDCLASSEXW windowClass = {0};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.lpfnWndProc = windowProcedure;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH) COLOR_WINDOW;
	windowClass.lpszClassName = CLASS_HACKONTROL_REMOTE;

	if(!RegisterClassExW(&windowClass)) {
		KHWin32DialogErrorW(GetLastError(), L"RegisterClassExW");
		return FALSE;
	}

	LOG("[Remote]: Initializing the main window\n");
	window = CreateWindowExW(WS_EX_TOPMOST, CLASS_HACKONTROL_REMOTE, L"Remote", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, NULL, NULL, instance, NULL);

	if(!window) {
		KHWin32DialogErrorW(GetLastError(), L"CreateWindowExW");
		return FALSE;
	}

	titledBorder = CreateWindowExW(WS_EX_NOPARENTNOTIFY, L"Button", L"Connected Devices", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 5, 0, 0, 0, window, NULL, NULL, NULL);

	if(!titledBorder) {
		KHWin32DialogErrorW(GetLastError(), L"CreateWindowExW");
		return FALSE;
	}

	INITCOMMONCONTROLSEX controls;
	controls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	controls.dwICC = ICC_LISTVIEW_CLASSES;

	if(!InitCommonControlsEx(&controls)) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"InitCommonControlsEx");
		return FALSE;
	}

	listView = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | LVS_REPORT | LVS_SINGLESEL, 0, 0, 0, 0, window, NULL, NULL, NULL);

	if(!listView) {
		KHWin32DialogErrorW(GetLastError(), L"CreateWindowExW");
		return FALSE;
	}

	SendMessageW(listView, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	LVCOLUMNW column = {0};
	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	column.cx = (int) (((double) GetSystemMetrics(SM_CXSCREEN)) * 0.13250366);
	column.fmt = LVCFMT_LEFT;
	column.pszText = L"IP Address";

	if(SendMessageW(listView, LVM_INSERTCOLUMN, 0, (LPARAM) &column) == -1) {
		KHWin32DialogErrorW(GetLastError(), L"ListView_InsertColumn");
		return FALSE;
	}

	column.pszText = L"Username";

	if(SendMessageW(listView, LVM_INSERTCOLUMN, 0, (LPARAM) &column) == 1) {
		KHWin32DialogErrorW(GetLastError(), L"ListView_InsertColumn");
		return FALSE;
	}

	return TRUE;
}

int MainWindowMessageLoop() {
	NONCLIENTMETRICS metrics;
	metrics.cbSize = sizeof(NONCLIENTMETRICS);

	if(!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0)) {
		KHWin32DialogErrorW(GetLastError(), L"SystemParametersInfoW");
		return 1;
	}

	HFONT font = CreateFontIndirectW(&metrics.lfCaptionFont);

	if(!font) {
		KHWin32DialogErrorW(GetLastError(), L"CreateFontIndirectW");
		return 1;
	}

	SendMessageW(titledBorder, WM_SETFONT, (WPARAM) font, TRUE);
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int width = (int) (((double) screenWidth) * 0.292825769);
	int height = (int) (((double) screenHeight) * 0.78125);
	int returnValue = 1;

	if(!SetWindowPos(window, HWND_TOPMOST, (screenWidth - width) / 2, (screenHeight - height) / 2, width, height, SWP_SHOWWINDOW)) {
		KHWin32DialogErrorW(GetLastError(), L"SetWindowPos");
		goto deleteFont;
	}

	LOG("[Remote]: Start message loop\n");
	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	returnValue = 0;
deleteFont:
	DeleteObject(font);
	return returnValue;
}

void MainWindowRefreshListView() {
	if(WaitForSingleObject(clientsLock, INFINITE) == WAIT_FAILED) {
		return;
	}

	SendMessageW(listView, LVM_DELETEALLITEMS, 0, 0);
	LVITEMW item = {0};
	item.mask = LVIF_TEXT;
	PCLIENT client;

	for(size_t i = 0; i < clients.elementCount; i++) {
		if(KHArrayGet(&clients, i, &client) && client->active) {
			item.iSubItem = 0;
			item.pszText = client ? client->name : L"(Missing name)";
			SendMessageW(listView, LVM_INSERTITEM, 0, (LPARAM) &item);
			item.iSubItem = 1;
			item.pszText = client ? client->address : L"(Missing address)";
			SendMessageW(listView, LVM_SETITEM, 0, (LPARAM) &item);
		}
	}

	ReleaseMutex(clientsLock);
}

void MainWindowExit() {
	PostMessageW(window, WM_CLOSE, 0, 0);
}
