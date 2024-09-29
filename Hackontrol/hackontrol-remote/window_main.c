#include "remote.h"
#include <CommCtrl.h>

static HWND window;
static HWND border;
static HWND listView;

int WindowMain(const HINSTANCE instance) {
	LOG("[Main Window]: Initializing\n");
	WNDCLASSEXW windowClass = {0};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.lpfnWndProc = DefWindowProcW;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH) COLOR_WINDOW;
	windowClass.lpszClassName = CLASS_HACKONTROL_REMOTE;

	if(!RegisterClassExW(&windowClass)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"RegisterClassExW");
		return 1;
	}

	int codeExit = 1;
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int width = (int) (screenWidth * 0.292825769);
	int height  = (int) (screenHeight * 0.78125);
	window = CreateWindowExW(WS_EX_TOPMOST, CLASS_HACKONTROL_REMOTE, L"Remote", WS_OVERLAPPEDWINDOW, (screenWidth - width) / 2, (screenHeight - height) / 2, width, height, NULL, NULL, instance, NULL);

	if(!window) {
		KHOPANLASTERRORMESSAGE_WIN32(L"CreateWindowExW");
		goto functionExit;
	}

	border = CreateWindowExW(WS_EX_NOPARENTNOTIFY, L"Button", L"Connected Devices", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 5, 0, 0, 0, window, NULL, NULL, NULL);

	if(!border) {
		KHOPANLASTERRORMESSAGE_WIN32(L"CreateWindowExW");
		goto functionExit;
	}

	INITCOMMONCONTROLSEX controls;
	controls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	controls.dwICC = ICC_LISTVIEW_CLASSES;

	if(!InitCommonControlsEx(&controls)) {
		KHOPANERRORMESSAGE_WIN32(ERROR_FUNCTION_FAILED, L"InitCommonControlsEx");
		goto functionExit;
	}

	listView = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | LVS_REPORT | LVS_SINGLESEL, 0, 0, 0, 0, window, NULL, NULL, NULL);

	if(!listView) {
		KHOPANLASTERRORMESSAGE_WIN32(L"CreateWindowExW");
		goto functionExit;
	}

	SendMessageW(listView, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	LVCOLUMNW column = {0};
	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	column.cx = (int) (screenWidth * 0.13250366);
	column.fmt = LVCFMT_LEFT;
	column.pszText = L"IP Address";

	if(SendMessageW(listView, LVM_INSERTCOLUMN, 0, (LPARAM) &column) == -1) {
		KHOPANLASTERRORMESSAGE_WIN32(L"ListView_InsertColumn");
		goto functionExit;
	}

	column.pszText = L"Username";

	if(SendMessageW(listView, LVM_INSERTCOLUMN, 0, (LPARAM) &column) == 1) {
		KHOPANLASTERRORMESSAGE_WIN32(L"ListView_InsertColumn");
		goto functionExit;
	}

	ShowWindow(window, SW_NORMAL);
	LOG("[Main Window]: Finished\n");
	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	if(!UnregisterClassW(CLASS_HACKONTROL_REMOTE, instance)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"UnregisterClassW");
		goto functionExit;
	}
functionExit:
	return codeExit;
}

void WindowMainExit() {
	PostMessageW(window, WM_CLOSE, 0, 0);
}
