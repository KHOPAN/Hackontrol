#include "thread_client.h"
#include <khopanwin32.h>
#include <khopanarray.h>
#include <CommCtrl.h>
#include "window_main.h"
#include "logger.h"

#define IDM_REMOTE_OPEN          0xE001
#define IDM_REMOTE_DISCONNECT    0xE002
#define IDM_REMOTE_REFRESH       0xE003
#define IDM_REMOTE_ALWAYS_ON_TOP 0xE004
#define IDM_REMOTE_EXIT          0xE005

#pragma warning(disable: 26454)

extern HINSTANCE programInstance;
extern ArrayList clientList;

static HWND window;
static HWND titledBorder;
static HWND listView;

static LRESULT CALLBACK windowProcedure(_In_ HWND inputWindow, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	switch(message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_SIZE: {
		RECT bounds;
		GetClientRect(window, &bounds);
		SetWindowPos(titledBorder, HWND_TOP, 0, 0, bounds.right - bounds.left - 10, bounds.bottom - bounds.top - 4, SWP_NOMOVE);
		GetClientRect(titledBorder, &bounds);
		SetWindowPos(listView, HWND_TOP, bounds.left + 9, bounds.top + 17, bounds.right - bounds.left - 8, bounds.bottom - bounds.top - 22, 0);
		return 0;
	}
	case WM_CONTEXTMENU: {
		int x = LOWORD(lparam);
		int y = HIWORD(lparam);
		LVHITTESTINFO hitTest = {0};
		hitTest.pt.x = x;
		hitTest.pt.y = y;
		ScreenToClient(listView, &hitTest.pt);
		RECT bounds;
		GetClientRect(listView, &bounds);

		if(hitTest.pt.x < bounds.left || hitTest.pt.x > bounds.right || hitTest.pt.y < bounds.top || hitTest.pt.y > bounds.bottom) {
			return 0;
		}

		HMENU popupMenu = CreatePopupMenu();

		if(!popupMenu) {
			return 0;
		}

		PCLIENT client = NULL;

		if(SendMessageW(listView, LVM_HITTEST, 0, (LPARAM) &hitTest) != -1 && KHArrayGet(&clientList, hitTest.iItem, &client)) {
			InsertMenuW(popupMenu, -1, MF_BYPOSITION | MF_STRING, IDM_REMOTE_OPEN, L"Open");
			InsertMenuW(popupMenu, -1, MF_BYPOSITION | MF_STRING, IDM_REMOTE_DISCONNECT, L"Disconnect");
			InsertMenuW(popupMenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
		}

		InsertMenuW(popupMenu, -1, MF_BYPOSITION | MF_STRING, IDM_REMOTE_REFRESH, L"Refresh");
		InsertMenuW(popupMenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
		InsertMenuW(popupMenu, -1, MF_BYPOSITION | MF_STRING, IDM_REMOTE_ALWAYS_ON_TOP, L"Always On Top");
		InsertMenuW(popupMenu, -1, MF_BYPOSITION | MF_STRING, IDM_REMOTE_EXIT, L"Exit");
		SetForegroundWindow(window);
		BOOL response = TrackPopupMenuEx(popupMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, x, y, window, NULL);
		DestroyMenu(popupMenu);

		switch(response) {
		case IDM_REMOTE_OPEN:
			ClientOpen(client);
			break;
		case IDM_REMOTE_DISCONNECT:
			LOG("[Main Window]: Disconnecting %ws\n" COMMA client ? client->address : L"(Missing address)");
			ClientDisconnect(client);
			break;
		case IDM_REMOTE_REFRESH:
			LOG("[Main Window]: Refreshing list view\n");
			RefreshMainWindowListView();
			break;
		}

		return 0;
	}
	case WM_NOTIFY: {
		if(((LPNMHDR) lparam)->code != NM_DBLCLK) {
			return 0;
		}

		PCLIENT client;

		if(!KHArrayGet(&clientList, ((LPNMITEMACTIVATE) lparam)->iItem, &client)) {
			return 0;
		}

		ClientOpen(client);
	}
	}

	return DefWindowProcW(inputWindow, message, wparam, lparam);
}

BOOL InitializeMainWindow() {
	WNDCLASSEXW windowClass = {0};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.lpfnWndProc = windowProcedure;
	windowClass.hInstance = programInstance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH) COLOR_WINDOW;
	windowClass.lpszClassName = CLASS_HACKONTROL_REMOTE;

	if(!RegisterClassExW(&windowClass)) {
		KHWin32DialogErrorW(GetLastError(), L"RegisterClassExW");
		return FALSE;
	}

	LOG("[Hackontrol Remote]: Creating the main window\n");
	window = CreateWindowExW(0, CLASS_HACKONTROL_REMOTE, L"Hackontrol Remote", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, NULL, NULL, programInstance, NULL);

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
	column.cx = (int) (((double) GetSystemMetrics(SM_CXSCREEN)) * 0.109809663);
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

	LOG("[Hackontrol Remote]: Starting the message loop\n");
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

void RefreshMainWindowListView() {
	SendMessageW(listView, LVM_DELETEALLITEMS, 0, 0);
	LVITEMW item = {0};
	item.mask = LVIF_TEXT;

	for(size_t i = 0; i < clientList.elementCount; i++) {
		CLIENT* client = NULL;
		KHArrayGet(&clientList, i, &client);
		item.iSubItem = 0;
		item.pszText = client ? client->name : L"(Missing name)";
		SendMessageW(listView, LVM_INSERTITEM, 0, (LPARAM) &item);
		item.iSubItem = 1;
		item.pszText = client ? client->address : L"(Missing address)";
		SendMessageW(listView, LVM_SETITEM, 0, (LPARAM) &item);
	}
}

void ExitMainWindow() {
	PostMessageW(window, WM_CLOSE, 0, 0);
}
