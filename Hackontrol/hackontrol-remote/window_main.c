#include <libkhopanlist.h>
#include "remote.h"
#include <CommCtrl.h>

extern LINKEDLIST clientList;

static HWND window;
static HWND border;
static HWND listView;

static LRESULT CALLBACK windowProcedure(_In_ HWND inputWindow, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	RECT bounds;
	POINT point;
	int status;
	LVHITTESTINFO information = {0};

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
	/*case WM_CONTEXTMENU:
		GetCursorPos(&point);
		ScreenToClient(listView, &point);
		GetClientRect(window, &bounds);

		if(point.x < bounds.left || point.x > bounds.right || point.y < bounds.top || point.y > bounds.bottom) {
			return 0;
		}

		status = 0;

		if(SendMessageW(listView, LVM_HITTEST, 0, (LPARAM) &information) != -1 && WaitForSingleObject(clientsLock, INFINITE) != WAIT_FAILED) {
			status = activeItem(information.iItem, &client);
			ReleaseMutex(clientsLock);
		}

		if(status) {
			InsertMenuW(contextMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
			InsertMenuW(contextMenu, 0, MF_BYPOSITION, IDM_REMOTE_DISCONNECT, L"Disconnect");
			InsertMenuW(contextMenu, 0, MF_BYPOSITION, IDM_REMOTE_OPEN, L"Open");
		}

		SetForegroundWindow(window);
		status = TrackPopupMenuEx(contextMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, LOWORD(lparam), HIWORD(lparam), window, NULL);

		return 0;*/
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
	PLINKEDLISTITEM item;

	KHOPAN_LINKED_LIST_ITERATE(item, &clientList) {

	}
}

void WindowMainExit() {
	PostMessageW(window, WM_CLOSE, 0, 0);
}
