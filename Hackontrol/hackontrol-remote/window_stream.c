#include "remote.h"

#define IDM_STREAM_ENABLE            0xE001
#define IDM_SEND_METHOD_FULL         0xE002
#define IDM_SEND_METHOD_BOUNDARY     0xE003
#define IDM_SEND_METHOD_COLOR        0xE004
#define IDM_SEND_METHOD_UNCOMPRESSED 0xE005
#define IDM_ALWAYS_ON_TOP            0xE006
#define IDM_FULLSCREEN               0xE007
#define IDM_MATCH_ASPECT_RATIO       0xE008
#define IDM_PICTURE_IN_PICTURE       0xE009
#define IDM_LOCK_FRAME               0xE00A
#define IDM_LIMIT_TO_SCREEN          0xE00B
#define IDM_CLOSE_WINDOW             0xE00C

extern HINSTANCE instance;

static LRESULT CALLBACK windowProcedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	PCLIENT client = (PCLIENT) GetWindowLongPtrW(window, GWLP_USERDATA);

	if(!client) {
		if(message != WM_CREATE) {
			return DefWindowProcW(window, message, wparam, lparam);
		}

		client = (PCLIENT) (((CREATESTRUCT*) lparam)->lpCreateParams);
		SetWindowLongPtrW(window, GWLP_USERDATA, (LONG_PTR) client);
	}

	RECT bounds;
	POINT location;
	PAINTSTRUCT paintStruct = {0};
	HDC context;
	HDC memoryContext;
	HBITMAP bitmap;
	HBITMAP oldBitmap;
	HBRUSH brush;
	BITMAPINFO information = {0};
	HMENU menu;
	HMENU sendMethodMenu;

	switch(message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_SIZE:
		GetClientRect(window, &bounds);
		bounds.right -= bounds.left;
		bounds.bottom -= bounds.top;

		if(client->session.stream.sourceWidth < 1 || client->session.stream.sourceHeight < 1 || bounds.right < 1 || bounds.bottom < 1 || WaitForSingleObject(client->mutex, INFINITE) == WAIT_FAILED) {
			break;
		}

		client->session.stream.imageWidth = (int) (((double) client->session.stream.sourceWidth) / ((double) client->session.stream.sourceHeight) * ((double) bounds.bottom));
		client->session.stream.imageHeight = (int) (((double) client->session.stream.sourceHeight) / ((double) client->session.stream.sourceWidth) * ((double) bounds.right));
		location.x = client->session.stream.imageWidth < bounds.right;

		if(location.x) {
			client->session.stream.imageHeight = bounds.bottom;
		} else {
			client->session.stream.imageWidth = bounds.right;
		}

		client->session.stream.imageX = location.x ? (int) ((((double) bounds.right) - ((double) client->session.stream.imageWidth)) / 2.0) : 0;
		client->session.stream.imageY = location.x ? 0 : (int) ((((double) bounds.bottom) - ((double) client->session.stream.imageHeight)) / 2.0);
		ReleaseMutex(client->mutex);
		break;
	case WM_PAINT:
		GetClientRect(window, &bounds);
		context = BeginPaint(window, &paintStruct);
		memoryContext = CreateCompatibleDC(context);
		bitmap = CreateCompatibleBitmap(context, bounds.right, bounds.bottom);
		oldBitmap = SelectObject(memoryContext, bitmap);
		brush = GetStockObject(DC_BRUSH);
		SetDCBrushColor(memoryContext, 0x000000);
		FillRect(memoryContext, &bounds, brush);

		if(WaitForSingleObject(client->mutex, INFINITE) == WAIT_FAILED) {
			goto blit;
		}

		if(client->session.stream.pixels) {
			information.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			information.bmiHeader.biWidth = client->session.stream.sourceWidth;
			information.bmiHeader.biHeight = client->session.stream.sourceHeight;
			information.bmiHeader.biPlanes = 1;
			information.bmiHeader.biBitCount = 32;
			SetStretchBltMode(memoryContext, HALFTONE);
			StretchDIBits(memoryContext, client->session.stream.imageX, client->session.stream.imageY, client->session.stream.imageWidth, client->session.stream.imageHeight, 0, 0, client->session.stream.sourceWidth, client->session.stream.sourceHeight, client->session.stream.pixels, &information, DIB_RGB_COLORS, SRCCOPY);
		}

		ReleaseMutex(client->mutex);
	blit:
		BitBlt(context, 0, 0, bounds.right, bounds.bottom, memoryContext, 0, 0, SRCCOPY);
		SelectObject(memoryContext, oldBitmap);
		DeleteObject(bitmap);
		DeleteDC(memoryContext);
		EndPaint(window, &paintStruct);
		break;
	case WM_CONTEXTMENU:
		menu = CreatePopupMenu();

		if(!menu) {
			break;
		}

		sendMethodMenu = CreateMenu();

		if(!sendMethodMenu) {
			DestroyMenu(menu);
			break;
		}

		if(WaitForSingleObject(client->mutex, INFINITE) == WAIT_FAILED) {
			DestroyMenu(sendMethodMenu);
			DestroyMenu(menu);
			break;
		}

		AppendMenuW(menu, MF_STRING | (client->session.stream.menu.stream ? MF_CHECKED : MF_UNCHECKED), IDM_STREAM_ENABLE, L"Enable Streaming");
		AppendMenuW(sendMethodMenu, MF_STRING | (client->session.stream.menu.method == SEND_METHOD_FULL ? MF_CHECKED : MF_UNCHECKED), IDM_SEND_METHOD_FULL, L"Full");
		AppendMenuW(sendMethodMenu, MF_STRING | (client->session.stream.menu.method == SEND_METHOD_BOUNDARY ? MF_CHECKED : MF_UNCHECKED), IDM_SEND_METHOD_BOUNDARY, L"Boundary Differences");
		AppendMenuW(sendMethodMenu, MF_STRING | (client->session.stream.menu.method == SEND_METHOD_COLOR ? MF_CHECKED : MF_UNCHECKED), IDM_SEND_METHOD_COLOR, L"Color Differences");
		AppendMenuW(sendMethodMenu, MF_STRING | (client->session.stream.menu.method == SEND_METHOD_UNCOMPRESSED ? MF_CHECKED : MF_UNCHECKED), IDM_SEND_METHOD_UNCOMPRESSED, L"Uncompressed");
		AppendMenuW(menu, MF_POPUP | (client->session.stream.menu.stream ? MF_ENABLED : MF_DISABLED), (UINT_PTR) sendMethodMenu, L"Send Method");
		AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
		AppendMenuW(menu, MF_STRING | (client->session.stream.menu.fullscreen ? MF_DISABLED : MF_ENABLED), IDM_MATCH_ASPECT_RATIO, L"Match Aspect Ratio");
		AppendMenuW(menu, MF_STRING | (client->session.stream.menu.alwaysOnTop ? MF_CHECKED : MF_UNCHECKED), IDM_ALWAYS_ON_TOP, L"Always On Top");
		AppendMenuW(menu, MF_STRING | (client->session.stream.menu.fullscreen ? MF_CHECKED : MF_UNCHECKED), IDM_FULLSCREEN, L"Fullscreen");
		AppendMenuW(menu, MF_STRING | (client->session.stream.menu.limitToScreen ? MF_CHECKED : MF_UNCHECKED) | (client->session.stream.menu.fullscreen ? MF_DISABLED : MF_ENABLED), IDM_LIMIT_TO_SCREEN, L"Limit To Screen");
		AppendMenuW(menu, MF_STRING | (client->session.stream.menu.lockFrame ? MF_CHECKED : MF_UNCHECKED) | (client->session.stream.menu.fullscreen ? MF_DISABLED : MF_ENABLED), IDM_LOCK_FRAME, L"Lock Frame");
		AppendMenuW(menu, MF_STRING | (client->session.stream.menu.pictureInPicture ? MF_CHECKED : MF_UNCHECKED) | (client->session.stream.menu.fullscreen ? MF_DISABLED : MF_ENABLED), IDM_PICTURE_IN_PICTURE, L"Picture In Picture");
		AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
		AppendMenuW(menu, MF_STRING, IDM_CLOSE_WINDOW, L"Close Window");
		SetForegroundWindow(window);
		ReleaseMutex(client->mutex);
		TrackPopupMenuEx(menu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, LOWORD(lparam), HIWORD(lparam), window, NULL);
		DestroyMenu(sendMethodMenu);
		DestroyMenu(menu);
		break;
	}

	return DefWindowProcW(window, message, wparam, lparam);
}

BOOL WindowStreamInitialize() {
	WNDCLASSEXW windowClass = {0};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.lpfnWndProc = windowProcedure;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH) COLOR_WINDOW;
	windowClass.lpszClassName = CLASS_SESSION_STREAM;

	if(!RegisterClassExW(&windowClass)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"RegisterClassExW");
		return FALSE;
	}

	return TRUE;
}

DWORD WINAPI WindowStream(_In_ PCLIENT client) {
	if(!client) {
		LOG("[Stream]: Empty thread parameter\n");
		return 1;
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int width = (int) (((double) screenWidth) * 0.439238653);
	int height = (int) (((double) screenHeight) * 0.520833333);
	LPWSTR title = KHOPANFormatMessage(L"Stream [%ws]", client->name);
	client->session.stream.window = CreateWindowExW(WS_EX_TOPMOST, CLASS_SESSION_STREAM, title ? title : L"Stream", WS_OVERLAPPEDWINDOW | WS_VISIBLE, (screenWidth - width) / 2, (screenHeight - height) / 2, width, height, NULL, NULL, instance, client);

	if(title) {
		LocalFree(title);
	}

	DWORD codeExit = 1;

	if(!client->session.stream.window) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		goto functionExit;
	}

	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	codeExit = 0;
	DestroyWindow(client->session.stream.window);
	client->session.stream.window = NULL;
functionExit:
	LOG("[Stream %ws]: Exit with code: %d\n", client->address, codeExit);
	CloseHandle(client->session.stream.thread);
	client->session.stream.thread = NULL;
	return codeExit;
}

void WindowStreamFrame(const PCLIENT client, const PBYTE data, const size_t size) {
	if(!data || size < 9) {
		return;
	}

	BOOL boundaryDifference = data[0] & 1;
	BOOL colorDifference = (data[0] >> 1) & 1;
	int width = (data[1] << 24) | (data[2] << 16) | (data[3] << 8) | data[4];
	int height = (data[5] << 24) | (data[6] << 16) | (data[7] << 8) | data[8];

	if(client->session.stream.sourceWidth != width || client->session.stream.sourceHeight != height) {
		client->session.stream.sourceWidth = width;
		client->session.stream.sourceHeight = height;

		if(client->session.stream.pixels) {
			KHOPAN_DEALLOCATE(client->session.stream.pixels);
		}

		client->session.stream.pixels = KHOPAN_ALLOCATE(width * height * 4);

		if(KHOPAN_ALLOCATE_FAILED(client->session.stream.pixels)) {
			return;
		}

		PostMessageW(client->session.stream.window, WM_SIZE, 0, 0);
	}

	if(!client->session.stream.pixels) {
		return;
	}
}

void WindowStreamClose(const PCLIENT client) {
	if(client->session.stream.window) {
		PostMessageW(client->session.stream.window, WM_CLOSE, 0, 0);
	}
}
