#include <khopanwin32.h>
#include <CommCtrl.h>
#include "window_main.h"
#include "logger.h"

static HINSTANCE windowInstance;
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
	}

	return DefWindowProcW(inputWindow, message, wparam, lparam);
}

BOOL InitializeMainWindow(const HINSTANCE instance) {
	windowInstance = instance;
	WNDCLASSEXW windowClass = {0};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.lpfnWndProc = windowProcedure;
	windowClass.hInstance = windowInstance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH) COLOR_WINDOW;
	windowClass.lpszClassName = CLASS_HACKONTROL_REMOTE;

	if(!RegisterClassExW(&windowClass)) {
		KHWin32DialogErrorW(GetLastError(), L"RegisterClassExW");
		return FALSE;
	}

	LOG("[Hackontrol Remote]: Creating the main window\n");
	window = CreateWindowExW(0, CLASS_HACKONTROL_REMOTE, L"Hackontrol Remote", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, NULL, NULL, windowInstance, NULL);

	if(!window) {
		KHWin32DialogErrorW(GetLastError(), L"CreateWindowExW");
		return FALSE;
	}

	titledBorder = CreateWindowExW(0, L"Button", L"Connected Devices", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 5, 0, 0, 0, window, NULL, NULL, NULL);

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

	listView = CreateWindowExW(WS_EX_NOPARENTNOTIFY | WS_EX_CLIENTEDGE, WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | LVS_REPORT | LVS_SINGLESEL, 0, 0, 0, 0, window, NULL, NULL, NULL);

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

	if(!SetWindowPos(window, HWND_TOPMOST, (screenWidth - width) / 2, (screenHeight - height) / 2, width, height, SWP_SHOWWINDOW)) {
		KHWin32DialogErrorW(GetLastError(), L"SetWindowPos");
		DeleteObject(font);
		return 1;
	}

	LOG("[Hackontrol Remote]: Starting the message loop\n");
	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	DeleteObject(font);
	LOG("[Hackontrol Remote]: Exiting\n");
	return 0;
}
