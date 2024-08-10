#include "thread_window.h"
#include <khopanwin32.h>
#include <khopanstring.h>
#include "logger.h"

#define IDM_WINDOW_EXIT                     0xE001
#define IDM_WINDOW_STREAMING_ENABLE         0xE002
#define IDM_WINDOW_SEND_METHOD_FULL         0xE003
#define IDM_WINDOW_SEND_METHOD_BOUNDARY     0xE004
#define IDM_WINDOW_SEND_METHOD_COLOR        0xE005
#define IDM_WINDOW_SEND_METHOD_UNCOMPRESSED 0xE006

extern HINSTANCE programInstance;

static void paintWindow(HDC context, HWND window) {
	RECT bounds;
	GetClientRect(window, &bounds);
	HBRUSH brush = GetStockObject(DC_BRUSH);
	SetDCBrushColor(context, 0x0000FF);
	FillRect(context, &bounds, brush);
}

static LRESULT CALLBACK windowProcedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	PCLIENT client = (PCLIENT) GetWindowLongPtrW(window, GWLP_USERDATA);

	if(!client) {
		if(message != WM_CREATE) {
			return DefWindowProcW(window, message, wparam, lparam);
		}

		client = (PCLIENT) (((CREATESTRUCT*) lparam)->lpCreateParams);
		SetWindowLongPtrW(window, GWLP_USERDATA, (LONG_PTR) client);
	}

	switch(message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT: {
		PAINTSTRUCT paintStruct;
		paintWindow(BeginPaint(window, &paintStruct), window);
		EndPaint(window, &paintStruct);
		return 0;
	}
	case WM_CONTEXTMENU: {
		HMENU popupMenu = CreatePopupMenu();

		if(!popupMenu) {
			return 0;
		}

		HMENU streamingMenu = CreateMenu();

		if(!streamingMenu) {
			DestroyMenu(popupMenu);
			return 0;
		}

		HMENU sendMethod = CreateMenu();

		if(!sendMethod) {
			DestroyMenu(streamingMenu);
			DestroyMenu(popupMenu);
			return 0;
		}

		AppendMenuW(streamingMenu, MF_STRING | (client->streaming ? MF_CHECKED : MF_UNCHECKED), IDM_WINDOW_STREAMING_ENABLE, L"Enable");
		AppendMenuW(sendMethod, MF_STRING, IDM_WINDOW_SEND_METHOD_FULL, L"Full");
		AppendMenuW(sendMethod, MF_STRING, IDM_WINDOW_SEND_METHOD_BOUNDARY, L"Boundary Differences");
		AppendMenuW(sendMethod, MF_STRING, IDM_WINDOW_SEND_METHOD_COLOR, L"Color Differences");
		AppendMenuW(sendMethod, MF_STRING, IDM_WINDOW_SEND_METHOD_UNCOMPRESSED, L"Uncompressed");
		AppendMenuW(streamingMenu, MF_POPUP, (UINT_PTR) sendMethod, L"Send Method");
		AppendMenuW(popupMenu, MF_POPUP, (UINT_PTR) streamingMenu, L"Streaming");
		AppendMenuW(popupMenu, MF_STRING, IDM_WINDOW_EXIT, L"Exit");
		SetForegroundWindow(window);
		BOOL response = TrackPopupMenuEx(popupMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, LOWORD(lparam), HIWORD(lparam), window, NULL);
		DestroyMenu(sendMethod);
		DestroyMenu(streamingMenu);
		DestroyMenu(popupMenu);

		switch(response) {
		case IDM_WINDOW_EXIT:
			LOG("[Window Thread %ws]: Exiting\n" COMMA client->address);
			ClientDisconnect(client);
			break;
		case IDM_WINDOW_STREAMING_ENABLE:
			client->streaming = !client->streaming;
			break;
		}

		return 0;
	}
	}

	return DefWindowProcW(window, message, wparam, lparam);
}

BOOL WindowRegisterClass() {
	WNDCLASSEXW windowClass = {0};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.lpfnWndProc = windowProcedure;
	windowClass.hInstance = programInstance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.lpszClassName = CLASS_CLIENT_WINDOW;

	if(!RegisterClassExW(&windowClass)) {
		KHWin32DialogErrorW(GetLastError(), L"RegisterClassExW");
		return FALSE;
	}

	return TRUE;
}

DWORD WINAPI WindowThread(_In_ PCLIENT client) {
	if(!client) {
		LOG("[Window Thread]: Exiting with an error: No client structure provided\n");
		return 1;
	}

	LOG("[Window Thread %ws]: Hello from window thread\n" COMMA client->address);
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int width = (int) (((double) screenWidth) * 0.439238653);
	int height = (int) (((double) screenHeight) * 0.520833333);
	LPWSTR windowName = KHFormatMessageW(L"%ws [%ws]", client->name, client->address);
	client->clientWindow = CreateWindowExW(0L, CLASS_CLIENT_WINDOW, windowName ? windowName : L"Client Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, (screenWidth - width) / 2, (screenHeight - height) / 2, width, height, NULL, NULL, NULL, client);

	if(windowName) {
		LocalFree(windowName);
	}

	int returnValue = 1;

	if(!client->clientWindow) {
		KHWin32DialogErrorW(GetLastError(), L"CreateWindowExW");
		goto exit;
	}

	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	returnValue = 0;
exit:
	LOG("[Window Thread %ws]: Exiting the window thread (Exit code: %d)\n" COMMA client->address COMMA returnValue);
	CloseHandle(client->windowThread);
	return returnValue;
}

void ExitClientWindow(const PCLIENT client) {
	if(client->windowThread) {
		PostMessageW(client->clientWindow, WM_CLOSE, 0, 0);
	}
}
