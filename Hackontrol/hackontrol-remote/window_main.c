#include "remote.h"
#include <CommCtrl.h>
#include <windowsx.h>

#define IDM_REMOTE_OPEN          0xE001
#define IDM_REMOTE_DISCONNECT    0xE002
#define IDM_REMOTE_REFRESH       0xE003
#define IDM_REMOTE_ALWAYS_ON_TOP 0xE004
#define IDM_REMOTE_EXIT          0xE005

#pragma warning(disable: 26454)

extern HINSTANCE instance;
extern HFONT font;
extern LINKEDLIST clientList;
extern HANDLE clientListMutex;

static HWND window;
static HWND border;
static HWND listView;
static BOOL sortUsername;
static BOOL sortAscending;

static int CALLBACK compareList(PCLIENT first, PCLIENT second, LPARAM parameter) {
	if(!first || !second) {
		return 0;
	}

	int compareUsername = wcscmp(first->name, second->name);
	int compareAddress = wcscmp(first->name, second->name);
	return (sortUsername ? compareUsername ? compareUsername : compareAddress : compareAddress ? compareAddress : compareUsername) * (sortAscending ? 1 : -1);
}

static void clickHeader(const int index) {
	if(index < 0 || WaitForSingleObject(clientListMutex, INFINITE) == WAIT_FAILED) {
		return;
	}

	HWND header = (HWND) SendMessageW(listView, LVM_GETHEADER, 0, 0);

	if(!header) {
		ReleaseMutex(clientListMutex);
		return;
	}

	int count = (int) SendMessageW(header, HDM_GETITEMCOUNT, 0, 0);

	if(count < 1) {
		ReleaseMutex(clientListMutex);
		return;
	}

	HDITEMW item = {0};

	for(int i = 0; i < count; i++) {
		item.mask = HDI_FORMAT;
		item.fmt = 0;
		SendMessageW(header, HDM_GETITEM, i, (LPARAM) &item);

		if(i != index) {
			item.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
			goto setItem;
		}

		if(item.fmt & HDF_SORTUP) {
			item.fmt = (item.fmt & ~HDF_SORTUP) | HDF_SORTDOWN;
		} else if(item.fmt & HDF_SORTDOWN) {
			item.fmt = (item.fmt & ~HDF_SORTDOWN) | HDF_SORTUP;
		} else {
			item.fmt |= HDF_SORTUP;
		}

		sortAscending = item.fmt & HDF_SORTUP;
	setItem:
		SendMessageW(header, HDM_SETITEM, i, (LPARAM) &item);
	}

	sortUsername = index == 0;
	SendMessageW(listView, LVM_SORTITEMS, 0, (LPARAM) compareList);
	ReleaseMutex(clientListMutex);
}

static BOOL openClient(const int index) {
	if(index < 0 && WaitForSingleObject(clientListMutex, INFINITE) == WAIT_FAILED) {
		return FALSE;
	}

	LVITEMW listItem = {0};
	listItem.mask = LVIF_PARAM;
	listItem.iItem = index;

	if(SendMessageW(listView, LVM_GETITEM, 0, (LPARAM) &listItem)) {
		ReleaseMutex(clientListMutex);
		ThreadClientOpen((PCLIENT) listItem.lParam);
		return TRUE;
	}

	ReleaseMutex(clientListMutex);
	return FALSE;
}

static BOOL insertInternal(const PCLIENT client) {
	LVITEMW listItem = {0};
	listItem.mask = LVIF_PARAM;
	int size = (int) SendMessageW(listView, LVM_GETITEMCOUNT, 0, 0);
	int index;

	for(index = 0; index < size; index++) {
		listItem.iItem = size - index - 1;

		if(SendMessageW(listView, LVM_GETITEM, 0, (LPARAM) &listItem) && compareList(client, (PCLIENT) listItem.lParam, 0) > 0) {
			listItem.iItem++;
			break;
		}
	}

	listItem.mask = LVIF_PARAM | LVIF_TEXT;
	listItem.pszText = client->name ? client->name : L"(Missing name)";
	listItem.lParam = (LPARAM) client;
	index = (int) SendMessageW(listView, LVM_INSERTITEM, 0, (LPARAM) &listItem);

	if(index == -1) {
		KHOPANERRORCONSOLE_WIN32(ERROR_FUNCTION_FAILED, L"ListView_InsertItem");
		return FALSE;
	}

	listItem.mask = LVIF_TEXT;
	listItem.iSubItem = 1;
	listItem.pszText = client->address ? client->address : L"(Missing address)";

	if(!SendMessageW(listView, LVM_SETITEM, 0, (LPARAM) &listItem)) {
		KHOPANERRORCONSOLE_WIN32(ERROR_FUNCTION_FAILED, L"ListView_SetItem");
		SendMessageW(listView, LVM_DELETEITEM, index, 0);
		return FALSE;
	}

	return TRUE;
}

static LRESULT CALLBACK procedure(HWND inputWindow, UINT message, WPARAM wparam, LPARAM lparam) {
	RECT bounds;
	LVHITTESTINFO information = {0};
	HWND header;
	BOOL showItem = TRUE;
	LVITEMW item = {0};
	PCLIENT client = NULL;
	HMENU menu;
	BOOL topMost;
	BOOL status;
	PLINKEDLISTITEM listItem;

	switch(message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_CONTEXTMENU:
		GetClientRect(listView, &bounds);
		information.pt.x = GET_X_LPARAM(lparam);
		information.pt.y = GET_Y_LPARAM(lparam);
		ScreenToClient(listView, &information.pt);

		if(information.pt.x < bounds.left || information.pt.x > bounds.right || information.pt.y < bounds.top || information.pt.y > bounds.bottom) {
			break;
		}

		if(header = (HWND) SendMessageW(listView, LVM_GETHEADER, 0, 0)) {
			GetClientRect(header, &bounds);
			if(information.pt.x >= bounds.left && information.pt.x <= bounds.right && information.pt.y >= bounds.top && information.pt.y <= bounds.bottom) showItem = FALSE;
		}

		if(showItem && WaitForSingleObject(clientListMutex, INFINITE) != WAIT_FAILED) {
			if(SendMessageW(listView, LVM_HITTEST, 0, (LPARAM) &information) != -1) {
				item.mask = LVIF_PARAM;
				item.iItem = information.iItem;
				if(SendMessageW(listView, LVM_GETITEM, 0, (LPARAM) &item)) client = (PCLIENT) item.lParam;
			}

			ReleaseMutex(clientListMutex);
		}

		menu = CreatePopupMenu();

		if(!menu) {
			break;
		}

		if(client) {
			AppendMenuW(menu, MF_STRING, IDM_REMOTE_OPEN, L"Open");
			AppendMenuW(menu, MF_STRING, IDM_REMOTE_DISCONNECT, L"Disconnect");
			AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
		}

		AppendMenuW(menu, MF_STRING, IDM_REMOTE_REFRESH, L"Refresh");
		AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
		topMost = GetWindowLongW(window, GWL_EXSTYLE) & WS_EX_TOPMOST;
		AppendMenuW(menu, MF_STRING | (topMost ? MF_CHECKED : MF_UNCHECKED), IDM_REMOTE_ALWAYS_ON_TOP, L"Always On Top");
		AppendMenuW(menu, MF_STRING, IDM_REMOTE_EXIT, L"Exit");
		SetForegroundWindow(window);
		status = TrackPopupMenuEx(menu, TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_TOPALIGN, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), window, NULL);
		DestroyMenu(menu);

		switch(status) {
		case IDM_REMOTE_OPEN:
			ThreadClientOpen(client);
			return 0;
		case IDM_REMOTE_DISCONNECT:
			ThreadClientDisconnect(client);
			return 0;
		case IDM_REMOTE_REFRESH:
			if(WaitForSingleObject(clientListMutex, INFINITE) == WAIT_FAILED) return 0;
			SendMessageW(listView, LVM_DELETEALLITEMS, 0, 0);

			KHOPAN_LINKED_LIST_ITERATE(listItem, &clientList) {
				if(listItem->data) insertInternal((PCLIENT) listItem->data);
			}

			ReleaseMutex(clientListMutex);
			return 0;
		case IDM_REMOTE_ALWAYS_ON_TOP:
			SetWindowPos(window, topMost ? HWND_NOTOPMOST : HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
			return 0;
		case IDM_REMOTE_EXIT:
			WindowMainExit();
			return 0;
		}

		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_NOTIFY:
		if(!lparam) {
			break;
		}

		switch(((LPNMHDR) lparam)->code) {
		case LVN_COLUMNCLICK:
			clickHeader((UINT) ((LPNMLISTVIEW) lparam)->iSubItem);
			return 0;
		case NM_DBLCLK:
			openClient(((LPNMITEMACTIVATE) lparam)->iItem);
			return 0;
		}

		break;
	case WM_SIZE:
		GetClientRect(window, &bounds);
		bounds.right -= bounds.left;
		bounds.bottom -= bounds.top;
		SetWindowPos(border, HWND_TOP, 0, 0, bounds.right - 10, bounds.bottom - 4, SWP_NOMOVE);
		SetWindowPos(listView, HWND_TOP, 0, 0, bounds.right - 18, bounds.bottom - 26, SWP_NOMOVE);
		return 0;
	}

	return DefWindowProcW(inputWindow, message, wparam, lparam);
}

BOOL WindowMainInitialize() {
	LOG("[Main Window]: Initializing\n");
	WNDCLASSEXW windowClass = {0};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.lpfnWndProc = procedure;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH) (COLOR_MENU + 1);
	windowClass.lpszClassName = CLASS_REMOTE;

	if(!RegisterClassExW(&windowClass)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"RegisterClassExW");
		return FALSE;
	}

	double screenWidth = GetSystemMetrics(SM_CXSCREEN);
	double screenHeight = GetSystemMetrics(SM_CYSCREEN);
	double width = screenWidth * 0.292825769;
	double height = screenHeight * 0.78125;
	window = CreateWindowExW(WS_EX_TOPMOST, CLASS_REMOTE, L"Remote", WS_OVERLAPPEDWINDOW, (int) ((screenWidth - width) / 2.0), (int) ((screenHeight - height) / 2.0), (int) width, (int) height, NULL, NULL, instance, NULL);

	if(!window) {
		KHOPANLASTERRORMESSAGE_WIN32(L"CreateWindowExW");
		goto unregisterClass;
	}

	border = CreateWindowExW(WS_EX_NOPARENTNOTIFY, L"Button", L"Target List", BS_GROUPBOX | WS_CHILD | WS_VISIBLE, 5, 0, 0, 0, window, NULL, NULL, NULL);

	if(!border) {
		KHOPANLASTERRORMESSAGE_WIN32(L"CreateWindowExW");
		goto destroyWindow;
	}

	SendMessageW(border, WM_SETFONT, (WPARAM) font, TRUE);
	listView = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEW, L"", LVS_REPORT | LVS_SINGLESEL | WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_VSCROLL, 9, 17, 0, 0, window, NULL, NULL, NULL);

	if(!listView) {
		KHOPANLASTERRORMESSAGE_WIN32(L"CreateWindowExW");
		goto destroyWindow;
	}

	SendMessageW(listView, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);
	LVCOLUMNW column = {0};
	column.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	column.fmt = LVCFMT_LEFT;
	column.cx = (int) (screenWidth * 0.133);
	column.pszText = L"Username";

	if(SendMessageW(listView, LVM_INSERTCOLUMN, 0, (LPARAM) &column) == -1) {
		KHOPANLASTERRORMESSAGE_WIN32(L"ListView_InsertColumn");
		goto destroyWindow;
	}

	column.pszText = L"IP Address";

	if(SendMessageW(listView, LVM_INSERTCOLUMN, 1, (LPARAM) &column) == -1) {
		KHOPANLASTERRORMESSAGE_WIN32(L"ListView_InsertColumn");
		goto destroyWindow;
	}

	clickHeader(0);
	return TRUE;
destroyWindow:
	DestroyWindow(window);
unregisterClass:
	UnregisterClassW(CLASS_REMOTE, instance);
	return FALSE;
}

void WindowMain() {
	ShowWindow(window, SW_NORMAL);
	LOG("[Main Window]: Finished\n");
	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		if(message.message == WM_KEYDOWN && (message.wParam == VK_RETURN || message.wParam == VK_SPACE) && openClient((int) SendMessageW(listView, LVM_GETNEXTITEM, -1, LVNI_SELECTED))) {
			continue;
		}

		if(!IsDialogMessageW(window, &message)) {
			TranslateMessage(&message);
			DispatchMessageW(&message);
		}
	}
}

BOOL WindowMainAdd(const PPCLIENT inputClient, const PPLINKEDLISTITEM inputItem) {
	if(WaitForSingleObject(clientListMutex, INFINITE) == WAIT_FAILED) {
		KHOPANLASTERRORCONSOLE_WIN32(L"WaitForSingleObject");
		return FALSE;
	}

	PLINKEDLISTITEM item;

	if(!KHOPANLinkedAdd(&clientList, (PBYTE) *inputClient, &item)) {
		KHOPANLASTERRORCONSOLE_WIN32(L"KHOPANLinkedAdd");
		goto releaseMutex;
	}

	PCLIENT client = (PCLIENT) item->data;

	if(!client || !insertInternal(client)) {
		KHOPANLinkedRemove(item);
		goto releaseMutex;
	}

	KHOPAN_DEALLOCATE(*inputClient);
	*inputClient = client;
	*inputItem = item;
	ReleaseMutex(clientListMutex);
	return TRUE;
releaseMutex:
	ReleaseMutex(clientListMutex);
	return FALSE;
}

BOOL WindowMainRemove(const PLINKEDLISTITEM item) {
	if(WaitForSingleObject(clientListMutex, INFINITE) == WAIT_FAILED) {
		KHOPANLASTERRORCONSOLE_WIN32(L"WaitForSingleObject");
		return FALSE;
	}

	LVFINDINFOW information = {0};
	information.flags = LVFI_PARAM;
	information.lParam = (LPARAM) item->data;
	int index = (int) SendMessageW(listView, LVM_FINDITEM, -1, (LPARAM) &information);

	if(index >= 0) {
		SendMessageW(listView, LVM_DELETEITEM, index, 0);
	}

	index = KHOPANLinkedRemove(item);
	ReleaseMutex(clientListMutex);

	if(!index) {
		KHOPANLASTERRORCONSOLE_WIN32(L"KHOPANLinkedRemove");
	}

	return index;
}

void WindowMainDestroy() {
	DestroyWindow(window);
	UnregisterClassW(CLASS_REMOTE, instance);
}

void WindowMainExit() {
	PostMessageW(window, WM_CLOSE, 0, 0);
}
