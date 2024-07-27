#include "connection.h"
#include <khopanwin32.h>
#include <khopanarray.h>
#include <CommCtrl.h>

#define HACKONTROL_REMOTE L"HackontrolRemote"

#pragma warning(disable: 6001)

static ArrayList globalClientList;
static int globalExitCode;
static HWND globalWindow;
static HWND globalTitledBorder;
static HWND globalListView;
static HMENU globalPopupMenu;

static LRESULT CALLBACK hackontrolRemoteProcedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	switch(message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(globalExitCode);
		return 0;
	case WM_SIZE: {
		RECT bounds;
		GetClientRect(window, &bounds);
		SetWindowPos(globalTitledBorder, HWND_TOP, 0, 0, bounds.right - bounds.left - 10, bounds.bottom - bounds.top - 4, SWP_NOMOVE);
		GetClientRect(globalTitledBorder, &bounds);
		SetWindowPos(globalListView, HWND_TOP, bounds.left + 9, bounds.top + 17, bounds.right - bounds.left - 8, bounds.bottom - bounds.top - 22, 0);
		return 0;
	}
	case WM_NOTIFY: {
		LPNMHDR notification = (LPNMHDR) lparam;

		if(notification->code != (UINT) -5) {
			return 0;
		}

		POINT point;
		GetCursorPos(&point);
		LVHITTESTINFO hitTest = {0};
		hitTest.pt = point;
		ScreenToClient(notification->hwndFrom, &hitTest.pt);

		if(SendMessageW(notification->hwndFrom, LVM_HITTEST, 0, (LPARAM) &hitTest) == -1) {
			return 0;
		}

		LVITEMW listItem = {0};
		listItem.mask = LVIF_TEXT;
		listItem.iItem = hitTest.iItem;

		if(!SendMessageW(notification->hwndFrom, LVM_GETITEM, 0, (LPARAM) &listItem)) {
			return 0;
		}

		SetForegroundWindow(window);
		TrackPopupMenuEx(globalPopupMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, point.x, point.y, window, NULL);
		return 0;
	}
	}

	return DefWindowProcW(window, message, wparam, lparam);
}

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE previousInstance, _In_ LPSTR argument, _In_ int commandLineShow) {
	WNDCLASSEXW windowClass = {0};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.lpfnWndProc = hackontrolRemoteProcedure;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH) COLOR_WINDOW;
	windowClass.lpszClassName = HACKONTROL_REMOTE;

	if(!RegisterClassExW(&windowClass)) {
		KHWin32DialogErrorW(GetLastError(), L"RegisterClassExW");
		return 1;
	}

	int returnValue = 1;

	if(!KHArrayInitialize(&globalClientList, sizeof(CLIENTENTRY))) {
		KHWin32DialogErrorW(GetLastError(), L"KHArrayInitialize");
		goto unregisterWindowClass;
	}

	globalWindow = CreateWindowExW(0, HACKONTROL_REMOTE, L"Hackontrol Remote", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, NULL, NULL, instance, NULL);

	if(!globalWindow) {
		KHWin32DialogErrorW(GetLastError(), L"CreateWindowExW");
		goto freeGlobalClientList;
	}

	HANDLE serverThread = CreateThread(NULL, 0, ServerThread, NULL, 0, NULL);

	if(!serverThread) {
		RemoteError(GetLastError(), L"CreateThread");
		goto freeGlobalClientList;
	}

	globalTitledBorder = CreateWindowExW(0, L"Button", L"Connected Devices", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 5, 0, 0, 0, globalWindow, NULL, NULL, NULL);

	if(!globalTitledBorder) {
		RemoteError(GetLastError(), L"CreateWindowExW");
		goto closeServerThread;
	}

	INITCOMMONCONTROLSEX controls;
	controls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	controls.dwICC = ICC_LISTVIEW_CLASSES;

	if(!InitCommonControlsEx(&controls)) {
		RemoteError(ERROR_FUNCTION_FAILED, L"InitCommonControlsEx");
		goto closeServerThread;
	}

	globalListView = CreateWindowExW(WS_EX_NOPARENTNOTIFY | WS_EX_CLIENTEDGE, WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | LVS_REPORT | LVS_SINGLESEL, 0, 0, 0, 0, globalWindow, NULL, NULL, NULL);

	if(!globalListView) {
		RemoteError(GetLastError(), L"CreateWindowExW");
		goto closeServerThread;
	}

	SendMessageW(globalListView, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	LVCOLUMNW column = {0};
	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	column.cx = (int) (((double) screenWidth) * 0.109809663);
	column.fmt = LVCFMT_LEFT;
	column.pszText = L"IP Address";

	if(SendMessageW(globalListView, LVM_INSERTCOLUMN, 0, (LPARAM) &column) == -1) {
		RemoteError(GetLastError(), L"ListView_InsertColumn");
		goto closeServerThread;
	}

	column.pszText = L"Username";

	if(SendMessageW(globalListView, LVM_INSERTCOLUMN, 0, (LPARAM) &column) == 1) {
		RemoteError(GetLastError(), L"ListView_InsertColumn");
		goto closeServerThread;
	}

	NONCLIENTMETRICS metrics;
	metrics.cbSize = sizeof(NONCLIENTMETRICS);

	if(!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0)) {
		RemoteError(GetLastError(), L"SystemParametersInfoW");
		goto closeServerThread;
	}

	HFONT font = CreateFontIndirectW(&metrics.lfCaptionFont);

	if(!font) {
		RemoteError(GetLastError(), L"CreateFontIndirectW");
		goto closeServerThread;
	}

	SendMessageW(globalTitledBorder, WM_SETFONT, (WPARAM) font, TRUE);
	globalPopupMenu = CreatePopupMenu();

	if(!globalPopupMenu) {
		RemoteError(GetLastError(), L"CreatePopupMenu");
		goto deleteFont;
	}

	if(!InsertMenuW(globalPopupMenu, 0, MF_BYPOSITION | MF_STRING, 1, L"Open")) {
		RemoteError(GetLastError(), L"InsertMenuW");
		goto destroyMenu;
	}

	if(!InsertMenuW(globalPopupMenu, 1, MF_BYPOSITION | MF_STRING, 2, L"Disconnect")) {
		RemoteError(GetLastError(), L"InsertMenuW");
		goto destroyMenu;
	}

	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int width = (int) (((double) screenWidth) * 0.292825769);
	int height = (int) (((double) screenHeight) * 0.78125);

	if(!SetWindowPos(globalWindow, HWND_TOP, (screenWidth - width) / 2, (screenHeight - height) / 2, width, height, SWP_SHOWWINDOW)) {
		RemoteError(GetLastError(), L"SetWindowPos");
		goto destroyMenu;
	}

	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	returnValue |= globalExitCode;
destroyMenu:
	DestroyMenu(globalPopupMenu);
deleteFont:
	DeleteObject(font);
closeServerThread:
	CloseHandle(serverThread);
freeGlobalClientList:
	for(size_t i = 0; i < globalClientList.elementCount; i++) {
		CLIENTENTRY* entry;

		if(!KHArrayGet(&globalClientList, i, &entry)) {
			continue;
		}

		if(entry->username) {
			LocalFree(entry->username);
		}

		LocalFree(entry->address);

		if(entry->clientThread) {
			CloseHandle(entry->clientThread);
		}
	}

	KHArrayFree(&globalClientList);
unregisterWindowClass:
	if(!UnregisterClassW(HACKONTROL_REMOTE, instance)) {
		KHWin32DialogErrorW(GetLastError(), L"UnregisterClassW");
		return 1;
	}

	return returnValue;
}

void ExitRemote(int exitCode) {
	globalExitCode = exitCode;
	SendMessageW(globalWindow, WM_CLOSE, 0, 0);
}

void RemoteError(DWORD errorCode, const LPWSTR functionName) {
	LPWSTR message = KHWin32GetErrorMessageW(errorCode, functionName);
	MessageBoxW(globalWindow, message, L"Hackontrol Remote Error", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
	LocalFree(message);
	ExitRemote(errorCode);
}

void RemoteHandleConnection(SOCKET clientSocket, LPWSTR address) {
	CLIENTENTRY entry = {0};

	if(!KHArrayAdd(&globalClientList, &entry)) {
		KHWin32DialogErrorW(GetLastError(), L"KHArrayAdd");
		closesocket(clientSocket);
		LocalFree(address);
		return;
	}

	CLIENTENTRY* entryPointer;

	if(!KHArrayGet(&globalClientList, globalClientList.elementCount - 1, &entryPointer)) {
		KHWin32DialogErrorW(GetLastError(), L"KHArrayGet");
		closesocket(clientSocket);
		LocalFree(address);
		return;
	}

	entryPointer->clientSocket = clientSocket;
	entryPointer->address = address;
	HANDLE clientThread = CreateThread(NULL, 0, ClientThread, entryPointer, 0, NULL);

	if(!clientThread) {
		KHWin32DialogErrorW(GetLastError(), L"CreateThread");
		closesocket(clientSocket);
		LocalFree(address);
	}

	entryPointer->clientThread = clientThread;
}

void RemoteRefreshClientList() {
	SendMessageW(globalListView, LVM_DELETEALLITEMS, 0, 0);
	LVITEMW item = {0};
	item.mask = LVIF_TEXT;

	for(size_t i = 0; i < globalClientList.elementCount; i++) {
		CLIENTENTRY* entry;

		if(!KHArrayGet(&globalClientList, i, &entry) || !entry->username || !entry->address) {
			continue;
		}

		item.iSubItem = 0;
		item.pszText = entry->username;

		if(SendMessageW(globalListView, LVM_INSERTITEM, 0, (LPARAM) &item) == -1) {
			continue;
		}

		item.iSubItem = 1;
		item.pszText = entry->address;
		SendMessageW(globalListView, LVM_SETITEM, 0, (LPARAM) &item);
	}
}
