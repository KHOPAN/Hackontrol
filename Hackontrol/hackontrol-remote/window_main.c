#include <windowsx.h>
#include "remote.h"
#include <CommCtrl.h>

#define IDSM_REMOTE_ALWAYS_ON_TOP 0xE010

#define IDM_REMOTE_OPEN       0xE001
#define IDM_REMOTE_DISCONNECT 0xE002
#define IDM_REMOTE_REFRESH    0xE003

#define CLASS_NAME L"HackontrolRemote"

extern HINSTANCE instance;
extern HFONT font;
extern LINKEDLIST clientList;
extern HANDLE clientListMutex;

static HWND window;
static HWND border;
static HWND listView;
static struct {
	BOOLEAN username : 1;
	BOOLEAN ascending : 1;
} sort;

static int CALLBACK compare(const PCLIENT first, const PCLIENT second, const LPARAM parameter) {
	if(!first) {
		return second ? -1 : 0;
	} else if(!second) {
		return 1;
	}

	char compare = wcscmp(sort.username ? first->name : first->address, sort.username ? second->name : second->address);
	compare = compare ? compare : wcscmp(sort.username ? first->address : first->name, sort.username ? second->address : second->name);
	return sort.ascending ? compare : -compare;
}

static BOOLEAN insert(const PCLIENT client) {
	LVITEMW item = {0};
	int size = (int) SendMessageW(listView, LVM_GETITEMCOUNT, 0, 0);
	int index;

	for(index = size - 1; index >= 0; index--) {
		item.mask = LVIF_PARAM;
		item.iItem = index;

		if(SendMessageW(listView, LVM_GETITEM, 0, (LPARAM) &item) && compare(client, (PCLIENT) item.lParam, 0) > 0) {
			item.iItem++;
			break;
		}
	}

	item.mask = LVIF_PARAM | LVIF_TEXT;
	item.pszText = client->name ? client->name : L"(Missing name)";
	item.lParam = (LPARAM) client;
	index = (int) SendMessageW(listView, LVM_INSERTITEM, 0, (LPARAM) &item);

	if(index == -1) {
		KHOPANERRORCONSOLE_WIN32(ERROR_FUNCTION_FAILED, L"ListView_InsertItem");
		return FALSE;
	}

	item.mask = LVIF_TEXT;
	item.iSubItem = 1;
	item.pszText = client->address ? client->address : L"(Missing address)";

	if(!SendMessageW(listView, LVM_SETITEM, 0, (LPARAM) &item)) {
		KHOPANERRORCONSOLE_WIN32(ERROR_FUNCTION_FAILED, L"ListView_SetItem");
		SendMessageW(listView, LVM_DELETEITEM, index, 0);
		return FALSE;
	}

	return TRUE;
}

static void listHeader(const int index) {
	if(index < 0 || WaitForSingleObject(clientListMutex, INFINITE) == WAIT_FAILED) {
		return;
	}

	HWND header = (HWND) SendMessageW(listView, LVM_GETHEADER, 0, 0);

	if(!header) {
		goto releaseMutex;
	}

	int count = (int) SendMessageW(header, HDM_GETITEMCOUNT, 0, 0);

	if(count < 1) {
		goto releaseMutex;
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

		sort.ascending = item.fmt & HDF_SORTUP ? TRUE : FALSE;
	setItem:
		SendMessageW(header, HDM_SETITEM, i, (LPARAM) &item);
	}

	sort.username = index == 0;
	SendMessageW(listView, LVM_SORTITEMS, 0, (LPARAM) compare);
releaseMutex:
	ReleaseMutex(clientListMutex);
}

static BOOLEAN open(const int index) {
	if(index < 0 && WaitForSingleObject(clientListMutex, INFINITE) == WAIT_FAILED) {
		return FALSE;
	}

	LVITEMW item = {0};
	item.mask = LVIF_PARAM;
	item.iItem = index;

	if(SendMessageW(listView, LVM_GETITEM, 0, (LPARAM) &item)) {
		ReleaseMutex(clientListMutex);
		ThreadClientOpen((PCLIENT) item.lParam);
		return TRUE;
	}

	ReleaseMutex(clientListMutex);
	return FALSE;
}

static LRESULT CALLBACK procedure(_In_ HWND inputWindow, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	RECT bounds;
	LVHITTESTINFO information = {0};
	HWND header;
	BOOLEAN showItem = TRUE;
	LVITEMW item = {0};
	PCLIENT client = NULL;
	HMENU menu;
	int status;
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
			KHOPAN_LINKED_LIST_ITERATE_FORWARD(listItem, &clientList) if(listItem->data) insert((PCLIENT) listItem->data);
			ReleaseMutex(clientListMutex);
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
			listHeader((UINT) ((LPNMLISTVIEW) lparam)->iSubItem);
			return 0;
		case NM_DBLCLK:
			open(((LPNMITEMACTIVATE) lparam)->iItem);
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
	case WM_SYSCOMMAND:
		if((wparam & 0xFFF0) == IDSM_REMOTE_ALWAYS_ON_TOP) {
			status = GetWindowLongW(window, GWL_EXSTYLE) & WS_EX_TOPMOST;
			CheckMenuItem(GetSystemMenu(window, FALSE), IDSM_REMOTE_ALWAYS_ON_TOP, MF_BYCOMMAND | (status ? MF_UNCHECKED : MF_CHECKED));
			SetWindowPos(window, status ? HWND_NOTOPMOST : HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
			return 0;
		}

		break;
	}

	return DefWindowProcW(inputWindow, message, wparam, lparam);
}

BOOLEAN WindowMainInitialize() {
	LOG("[Main Window]: Initializing\n");
	WNDCLASSEXW windowClass = {0};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.lpfnWndProc = procedure;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH) (COLOR_MENU + 1);
	windowClass.lpszClassName = CLASS_NAME;

	if(!RegisterClassExW(&windowClass)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"RegisterClassExW");
		return FALSE;
	}

	double screenWidth = GetSystemMetrics(SM_CXSCREEN);
	double screenHeight = GetSystemMetrics(SM_CYSCREEN);
	double width = screenWidth * 0.292825769;
	double height = screenHeight * 0.78125;
	window = CreateWindowExW(WS_EX_TOPMOST, CLASS_NAME, L"Remote", WS_OVERLAPPEDWINDOW, (int) ((screenWidth - width) / 2.0), (int) ((screenHeight - height) / 2.0), (int) width, (int) height, NULL, NULL, instance, NULL);

	if(!window) {
		KHOPANLASTERRORMESSAGE_WIN32(L"CreateWindowExW");
		goto unregisterClass;
	}

	HMENU menu = GetSystemMenu(window, FALSE);

	if(!InsertMenuW(menu, SC_CLOSE, MF_BYCOMMAND | MF_CHECKED | MF_STRING, IDSM_REMOTE_ALWAYS_ON_TOP, L"Always On Top")) {
		KHOPANLASTERRORMESSAGE_WIN32(L"InsertMenuW");
		goto destroyWindow;
	}

	if(!InsertMenuW(menu, SC_CLOSE, MF_BYCOMMAND | MF_SEPARATOR, 0, NULL)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"InsertMenuW");
		goto destroyWindow;
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
	column.cx = (int) (screenWidth * 0.133235724);
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

	listHeader(0);
	return TRUE;
destroyWindow:
	DestroyWindow(window);
unregisterClass:
	UnregisterClassW(CLASS_NAME, instance);
	return FALSE;
}

void WindowMain() {
	ShowWindow(window, SW_NORMAL);
	LOG("[Main Window]: Finished\n");
	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		if(message.message == WM_KEYDOWN && (message.wParam == VK_RETURN || message.wParam == VK_SPACE) && open((int) SendMessageW(listView, LVM_GETNEXTITEM, -1, LVNI_SELECTED))) {
			continue;
		}

		if(!IsDialogMessageW(window, &message)) {
			TranslateMessage(&message);
			DispatchMessageW(&message);
		}
	}
}

BOOLEAN WindowMainAdd(const PPCLIENT inputClient, const PPLINKEDLISTITEM inputItem) {
	if(WaitForSingleObject(clientListMutex, INFINITE) == WAIT_FAILED) {
		KHOPANLASTERRORCONSOLE_WIN32(L"WaitForSingleObject");
		return FALSE;
	}

	PLINKEDLISTITEM item;
	KHOPANERROR error;
	BOOLEAN codeExit = FALSE;

	if(!KHOPANLinkedAdd(&clientList, (PBYTE) *inputClient, &item, &error)) {
		KHOPANERRORCONSOLE_KHOPAN(error);
		goto releaseMutex;
	}

	PCLIENT client = (PCLIENT) item->data;

	if(!client || !insert(client)) {
		KHOPANLinkedRemove(item, NULL);
		goto releaseMutex;
	}

	KHOPAN_DEALLOCATE(*inputClient);
	*inputClient = client;
	*inputItem = item;
	codeExit = TRUE;
releaseMutex:
	ReleaseMutex(clientListMutex);
	return codeExit;
}

BOOLEAN WindowMainRemove(const PLINKEDLISTITEM item) {
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

	KHOPANERROR error;
	index = KHOPANLinkedRemove(item, &error);
	ReleaseMutex(clientListMutex);

	if(!index) {
		KHOPANERRORCONSOLE_KHOPAN(error);
	}

	return index;
}

void WindowMainExit() {
	PostMessageW(window, WM_CLOSE, 0, 0);
}

void WindowMainDestroy() {
	DestroyWindow(window);
	UnregisterClassW(CLASS_NAME, instance);
}
