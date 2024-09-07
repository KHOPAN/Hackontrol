#include "thread_window.h"
#include <khopanwin32.h>
#include <khopanstring.h>
#include <hackontrolpacket.h>
#include "logger.h"

#define CLASS_CLIENT_WINDOW L"HackontrolRemoteClientWindow"

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
#define IDM_DISCONNECT               0xE00D

static void sendFrameCode(const PCLIENT client) {
	BYTE data = ((client->window->menu.method & 0b11) << 1) | (client->window->menu.stream ? 0b1001 : 0);
	PACKET packet;
	packet.size = 1;
	packet.packetType = PACKET_TYPE_STREAM_FRAME;
	packet.data = &data;
	SendPacket(client->socket, &packet);
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

	POINT position;
	RECT bounds;
	PAINTSTRUCT paintStruct;
	HDC context;
	HDC memoryContext;
	HBITMAP bitmap;

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

		if(bounds.right < 1 || bounds.bottom < 1 || WaitForSingleObject(client->window->lock, INFINITE) == WAIT_FAILED) {
			break;
		}

		client->window->stream.imageWidth = (int) (((double) client->window->stream.originalImageWidth) / ((double) client->window->stream.originalImageHeight) * ((double) bounds.bottom));
		client->window->stream.imageHeight = (int) (((double) client->window->stream.originalImageHeight) / ((double) client->window->stream.originalImageWidth) * ((double) bounds.right));
		position.x = client->window->stream.imageWidth < bounds.right;

		if(position.x) {
			client->window->stream.imageHeight = bounds.bottom;
		} else {
			client->window->stream.imageWidth = bounds.right;
		}

		client->window->stream.imageX = position.x ? (int) ((((double) bounds.right) - ((double) client->window->stream.imageWidth)) / 2.0) : 0;
		client->window->stream.imageY = position.x ? 0 : (int) ((((double) bounds.bottom) - ((double) client->window->stream.imageHeight)) / 2.0);
		ReleaseMutex(client->window->lock);
		break;
	case WM_PAINT:
		GetClientRect(window, &bounds);
		context = BeginPaint(window, &paintStruct);
		memoryContext = CreateCompatibleDC(context);
		bitmap = CreateCompatibleBitmap(context, bounds.right, bounds.bottom);
		HBITMAP oldBitmap = SelectObject(memoryContext, bitmap);
		HBRUSH brush = GetStockObject(DC_BRUSH);
		SetDCBrushColor(memoryContext, 0x000000);
		FillRect(memoryContext, &bounds, brush);

		if(WaitForSingleObject(client->window->lock, INFINITE) == WAIT_FAILED) {
			goto blit;
		}

		if(client->window->stream.pixels) {
			BITMAPINFO information = {0};
			information.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			information.bmiHeader.biWidth = client->window->stream.originalImageWidth;
			information.bmiHeader.biHeight = client->window->stream.originalImageHeight;
			information.bmiHeader.biPlanes = 1;
			information.bmiHeader.biBitCount = 32;
			SetStretchBltMode(memoryContext, HALFTONE);
			StretchDIBits(memoryContext, client->window->stream.imageX, client->window->stream.imageY, client->window->stream.imageWidth, client->window->stream.imageHeight, 0, 0, client->window->stream.originalImageWidth, client->window->stream.originalImageHeight, client->window->stream.pixels, &information, DIB_RGB_COLORS, SRCCOPY);
		}

		ReleaseMutex(client->window->lock);
	blit:
		BitBlt(context, 0, 0, bounds.right, bounds.bottom, memoryContext, 0, 0, SRCCOPY);
		SelectObject(memoryContext, oldBitmap);
		DeleteObject(bitmap);
		DeleteDC(memoryContext);
		EndPaint(window, &paintStruct);
		break;
	case WM_ERASEBKGND:
		return 1;
	case WM_CONTEXTMENU:
		context = (HDC) CreatePopupMenu();

		if(!context) {
			break;
		}

		memoryContext = (HDC) CreateMenu();

		if(!memoryContext) {
			DestroyMenu((HMENU) context);
			break;
		}

		bitmap = (HBITMAP) CreateMenu();

		if(!bitmap || WaitForSingleObject(client->window->lock, INFINITE) == WAIT_FAILED) {
			DestroyMenu((HMENU) memoryContext);
			DestroyMenu((HMENU) context);
			break;
		}

		AppendMenuW((HMENU) memoryContext, MF_STRING | (client->window->menu.stream ? MF_CHECKED : MF_UNCHECKED), IDM_STREAM_ENABLE, L"Enable");
		AppendMenuW((HMENU) bitmap, MF_STRING | (client->window->menu.method == SEND_METHOD_FULL         ? MF_CHECKED : MF_UNCHECKED), IDM_SEND_METHOD_FULL,         L"Full");
		AppendMenuW((HMENU) bitmap, MF_STRING | (client->window->menu.method == SEND_METHOD_BOUNDARY     ? MF_CHECKED : MF_UNCHECKED), IDM_SEND_METHOD_BOUNDARY,     L"Boundary Differences");
		AppendMenuW((HMENU) bitmap, MF_STRING | (client->window->menu.method == SEND_METHOD_COLOR        ? MF_CHECKED : MF_UNCHECKED), IDM_SEND_METHOD_COLOR,        L"Color Differences");
		AppendMenuW((HMENU) bitmap, MF_STRING | (client->window->menu.method == SEND_METHOD_UNCOMPRESSED ? MF_CHECKED : MF_UNCHECKED), IDM_SEND_METHOD_UNCOMPRESSED, L"Uncompressed");
		AppendMenuW((HMENU) memoryContext, MF_POPUP | (client->window->menu.stream ? MF_ENABLED : MF_DISABLED), (UINT_PTR) bitmap, L"Send Method");
		AppendMenuW((HMENU) context, MF_POPUP, (UINT_PTR) memoryContext, L"Streaming");
		AppendMenuW((HMENU) context, MF_STRING | (client->window->menu.alwaysOnTop ? MF_CHECKED : MF_UNCHECKED),                                                       IDM_ALWAYS_ON_TOP,      L"Always On Top");
		AppendMenuW((HMENU) context, MF_STRING | (client->window->menu.fullscreen  ? MF_CHECKED : MF_UNCHECKED),                                                       IDM_FULLSCREEN,         L"Fullscreen");
		AppendMenuW((HMENU) context, MF_STRING | (client->window->menu.fullscreen  ? MF_DISABLED : client->window->menu.matchAspectRatio ? MF_CHECKED : MF_UNCHECKED), IDM_MATCH_ASPECT_RATIO, L"Match Aspect Ratio");
		AppendMenuW((HMENU) context, MF_SEPARATOR, 0, NULL);
		AppendMenuW((HMENU) context, MF_STRING | (client->window->menu.pictureInPicture ? MF_CHECKED : MF_UNCHECKED) | (client->window->menu.fullscreen ? MF_DISABLED : MF_ENABLED),                                                       IDM_PICTURE_IN_PICTURE, L"Picture In Picture");
		AppendMenuW((HMENU) context, MF_STRING | (client->window->menu.lockFrame        ? MF_CHECKED : MF_UNCHECKED) | (client->window->menu.fullscreen ? MF_DISABLED : client->window->menu.pictureInPicture ? MF_ENABLED : MF_DISABLED), IDM_LOCK_FRAME,         L"Lock Frame");
		AppendMenuW((HMENU) context, MF_STRING | (client->window->menu.limitToScreen    ? MF_CHECKED : MF_UNCHECKED) | (client->window->menu.fullscreen ? MF_DISABLED : client->window->menu.pictureInPicture ? MF_ENABLED : MF_DISABLED), IDM_LIMIT_TO_SCREEN,    L"Limit To Screen");
		AppendMenuW((HMENU) context, MF_SEPARATOR, 0, NULL);
		AppendMenuW((HMENU) context, MF_STRING, IDM_CLOSE_WINDOW, L"Close Window");
		AppendMenuW((HMENU) context, MF_STRING, IDM_DISCONNECT,   L"Disconnect");
		SetForegroundWindow(window);
		ReleaseMutex(client->window->lock);
		TrackPopupMenuEx((HMENU) context, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, LOWORD(lparam), HIWORD(lparam), window, NULL);
		DestroyMenu((HMENU) bitmap);
		DestroyMenu((HMENU) memoryContext);
		DestroyMenu((HMENU) context);
		break;
	case WM_COMMAND:
		if(WaitForSingleObject(client->window->lock, INFINITE) == WAIT_FAILED) {
			break;
		}

		switch(LOWORD(wparam)) {
		case IDM_STREAM_ENABLE:
			client->window->menu.stream = !client->window->menu.stream;
			sendFrameCode(client);
			break;
		case IDM_SEND_METHOD_FULL:
			client->window->menu.method = SEND_METHOD_FULL;
			sendFrameCode(client);
			break;
		case IDM_SEND_METHOD_BOUNDARY:
			client->window->menu.method = SEND_METHOD_BOUNDARY;
			sendFrameCode(client);
			break;
		case IDM_SEND_METHOD_COLOR:
			client->window->menu.method = SEND_METHOD_COLOR;
			sendFrameCode(client);
			break;
		case IDM_SEND_METHOD_UNCOMPRESSED:
			client->window->menu.method = SEND_METHOD_UNCOMPRESSED;
			sendFrameCode(client);
			break;
		case IDM_ALWAYS_ON_TOP:
			client->window->menu.alwaysOnTop = !client->window->menu.alwaysOnTop;
			SetWindowPos(window, client->window->menu.alwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
			break;
		case IDM_FULLSCREEN:
			client->window->menu.fullscreen = !client->window->menu.fullscreen;
			if(client->window->menu.fullscreen) client->window->storage.style = GetWindowLongPtrW(window, GWL_STYLE);
			client->window->storage.placement.length = sizeof(WINDOWPLACEMENT);
			(client->window->menu.fullscreen ? GetWindowPlacement : SetWindowPlacement)(window, &client->window->storage.placement);
			SetWindowLongPtrW(window, GWL_STYLE, client->window->menu.fullscreen ? WS_POPUP | WS_VISIBLE : client->window->storage.style);
			if(client->window->menu.fullscreen) SetWindowPos(window, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED);
			PostMessageW(window, WM_SIZE, 0, 0);
			break;
		case IDM_MATCH_ASPECT_RATIO:
			client->window->menu.matchAspectRatio = !client->window->menu.matchAspectRatio;
			if(!client->window->menu.matchAspectRatio) break;
			break;
		case IDM_PICTURE_IN_PICTURE:
			client->window->menu.pictureInPicture = !client->window->menu.pictureInPicture;
			SetWindowLongPtrW(window, GWL_STYLE, (client->window->menu.pictureInPicture ? WS_POPUP : WS_OVERLAPPEDWINDOW) | WS_VISIBLE);
			SetWindowPos(window, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
			PostMessageW(window, WM_SIZE, 0, 0);
			break;
		case IDM_LOCK_FRAME:
			client->window->menu.lockFrame = !client->window->menu.lockFrame;
			break;
		case IDM_LIMIT_TO_SCREEN:
			client->window->menu.limitToScreen = !client->window->menu.limitToScreen;
			if(!client->window->menu.limitToScreen) break;
			GetWindowRect(window, &bounds);
			bounds.right -= bounds.left;
			bounds.bottom -= bounds.top;
			position.x = GetSystemMetrics(SM_CXSCREEN);
			position.y = GetSystemMetrics(SM_CYSCREEN);
			bounds.left = max(bounds.left, 0);
			bounds.top = max(bounds.top, 0);
			bounds.right = min(bounds.right, position.x);
			bounds.bottom = min(bounds.bottom, position.y);
			if(bounds.left + bounds.right > position.x) bounds.left = position.x - bounds.right;
			if(bounds.top + bounds.bottom > position.y) bounds.top = position.y - bounds.bottom;
			SetWindowPos(window, HWND_TOP, bounds.left, bounds.top, bounds.right, bounds.bottom, 0);
			break;
		case IDM_CLOSE_WINDOW:
			PostMessageW(window, WM_CLOSE, 0, 0);
			break;
		case IDM_DISCONNECT:
			LOG("[Window %ws]: Disconnecting\n" COMMA client->address);
			ClientDisconnect(client);
			break;
		}

		ReleaseMutex(client->window->lock);
		break;
	case WM_MOUSEMOVE:
		if(WaitForSingleObject(client->window->lock, INFINITE) == WAIT_FAILED) {
			break;
		}

		if(client->window->menu.lockFrame || client->window->menu.fullscreen) {
			SetCursor(LoadCursorW(NULL, IDC_ARROW));
			ReleaseMutex(client->window->lock);
			break;
		}

		/*paintStruct.rcPaint.left = GetSystemMetrics(SM_CXSCREEN);
		paintStruct.rcPaint.top = GetSystemMetrics(SM_CYSCREEN);
		GetCursorPos(&position);

		if(!client->window->menu.pictureInPicture) {
			if(wparam != MK_LBUTTON) {
				ReleaseMutex(client->window->lock);
				break;
			}

			SetCursor(LoadCursorW(NULL, IDC_ARROW));
			position.x -= client->window->stream.position.x - client->window->stream.bounds.left;
			position.y -= client->window->stream.position.y - client->window->stream.bounds.top;
			SetWindowPos(window, HWND_TOP, client->window->menu.limitToScreen ? position.x < 0 ? 0 : position.x + client->window->stream.bounds.right - client->window->stream.bounds.left > paintStruct.rcPaint.left ? paintStruct.rcPaint.left - client->window->stream.bounds.right + client->window->stream.bounds.left : position.x : position.x, client->window->menu.limitToScreen ? position.y < 0 ? 0 : position.y + client->window->stream.bounds.bottom - client->window->stream.bounds.top > paintStruct.rcPaint.top ? paintStruct.rcPaint.top - client->window->stream.bounds.bottom + client->window->stream.bounds.top : position.y : position.y, 0, 0, SWP_NOSIZE);
			ReleaseMutex(client->window->lock);
			break;
		}*/

		paintStruct.rcPaint.left = GetSystemMetrics(SM_CXSCREEN);
		paintStruct.rcPaint.top = GetSystemMetrics(SM_CYSCREEN);
		GetCursorPos(&position);

		if(wparam != MK_LBUTTON) {
			position.x = LOWORD(lparam);
			position.y = HIWORD(lparam);
			GetClientRect(window, &bounds);
			client->window->stream.cursorNorth = position.y >= 0 && position.y <= client->window->stream.resizeActivationDistance;
			client->window->stream.cursorEast = position.x >= bounds.right - client->window->stream.resizeActivationDistance && position.x < bounds.right;
			client->window->stream.cursorSouth = position.y >= bounds.bottom - client->window->stream.resizeActivationDistance && position.y < bounds.bottom;
			client->window->stream.cursorWest = position.x >= 0 && position.x <= client->window->stream.resizeActivationDistance;
			SetCursor(LoadCursorW(NULL, client->window->stream.cursorNorth ? client->window->stream.cursorWest ? IDC_SIZENWSE : client->window->stream.cursorEast ? IDC_SIZENESW : IDC_SIZENS : client->window->stream.cursorSouth ? client->window->stream.cursorWest ? IDC_SIZENESW : client->window->stream.cursorEast ? IDC_SIZENWSE : IDC_SIZENS : client->window->stream.cursorWest ? IDC_SIZEWE : client->window->stream.cursorEast ? IDC_SIZEWE : IDC_ARROW));
			ReleaseMutex(client->window->lock);
			break;
		}

		if(!client->window->stream.cursorNorth && !client->window->stream.cursorEast && !client->window->stream.cursorSouth && !client->window->stream.cursorWest) {
			position.x -= client->window->stream.position.x - client->window->stream.bounds.left;
			position.y -= client->window->stream.position.y - client->window->stream.bounds.top;
			SetWindowPos(window, HWND_TOP, client->window->menu.limitToScreen ? position.x < 0 ? 0 : position.x + client->window->stream.bounds.right - client->window->stream.bounds.left > paintStruct.rcPaint.left ? paintStruct.rcPaint.left - client->window->stream.bounds.right + client->window->stream.bounds.left : position.x : position.x, client->window->menu.limitToScreen ? position.y < 0 ? 0 : position.y + client->window->stream.bounds.bottom - client->window->stream.bounds.top > paintStruct.rcPaint.top ? paintStruct.rcPaint.top - client->window->stream.bounds.bottom + client->window->stream.bounds.top : position.y : position.y, 0, 0, SWP_NOSIZE);
			ReleaseMutex(client->window->lock);
			break;
		}

		paintStruct.rcPaint.right = client->window->stream.resizeActivationDistance * 3;
		GetWindowRect(window, &bounds);
		bounds.right -= bounds.left;
		bounds.bottom -= bounds.top;

		if(client->window->stream.cursorNorth) {
			paintStruct.rcPaint.bottom = position.y - client->window->stream.position.y;
			if(client->window->menu.limitToScreen && client->window->stream.bounds.top + paintStruct.rcPaint.bottom < 0) paintStruct.rcPaint.bottom = -client->window->stream.bounds.top;
			if(client->window->stream.bounds.bottom - client->window->stream.bounds.top - paintStruct.rcPaint.bottom < paintStruct.rcPaint.right) paintStruct.rcPaint.bottom = client->window->stream.bounds.bottom - client->window->stream.bounds.top - paintStruct.rcPaint.right;
			bounds.top = client->window->stream.bounds.top + paintStruct.rcPaint.bottom;
			bounds.bottom = client->window->stream.bounds.bottom - bounds.top;
		}

		if(client->window->stream.cursorEast) {
			paintStruct.rcPaint.bottom = position.x - client->window->stream.position.x;
			if(client->window->menu.limitToScreen && client->window->stream.bounds.right + paintStruct.rcPaint.bottom > paintStruct.rcPaint.left) paintStruct.rcPaint.bottom = paintStruct.rcPaint.left - client->window->stream.bounds.right;
			bounds.right = client->window->stream.bounds.right - client->window->stream.bounds.left + paintStruct.rcPaint.bottom;
			bounds.right = max(bounds.right, paintStruct.rcPaint.right);
		}

		if(client->window->stream.cursorSouth) {
			paintStruct.rcPaint.bottom = position.y - client->window->stream.position.y;
			if(client->window->menu.limitToScreen && client->window->stream.bounds.bottom + paintStruct.rcPaint.bottom > paintStruct.rcPaint.top) paintStruct.rcPaint.bottom = paintStruct.rcPaint.top - client->window->stream.bounds.bottom;
			bounds.bottom = client->window->stream.bounds.bottom - client->window->stream.bounds.top + paintStruct.rcPaint.bottom;
			bounds.bottom = max(bounds.bottom, paintStruct.rcPaint.right);
		}

		if(client->window->stream.cursorWest) {
			paintStruct.rcPaint.bottom = position.x - client->window->stream.position.x;
			if(client->window->menu.limitToScreen && client->window->stream.bounds.left + paintStruct.rcPaint.bottom < 0) paintStruct.rcPaint.bottom = -client->window->stream.bounds.left;
			if(client->window->stream.bounds.right - client->window->stream.bounds.left - paintStruct.rcPaint.bottom < paintStruct.rcPaint.right) paintStruct.rcPaint.bottom = client->window->stream.bounds.right - client->window->stream.bounds.left - paintStruct.rcPaint.right;
			bounds.left = client->window->stream.bounds.left + paintStruct.rcPaint.bottom;
			bounds.right = client->window->stream.bounds.right - bounds.left;
		}

		SetWindowPos(window, HWND_TOP, bounds.left, bounds.top, bounds.right, bounds.bottom, (!client->window->stream.cursorNorth && client->window->stream.cursorEast) || (client->window->stream.cursorSouth && !client->window->stream.cursorWest) ? SWP_NOMOVE : 0);
		ReleaseMutex(client->window->lock);
		break;
	case WM_LBUTTONDOWN:
		if(WaitForSingleObject(client->window->lock, INFINITE) == WAIT_FAILED) {
			break;
		}

		GetWindowRect(window, &client->window->stream.bounds);
		GetCursorPos(&client->window->stream.position);
		ReleaseMutex(client->window->lock);
		SetCapture(window);
		break;
	case WM_LBUTTONUP:
		ReleaseCapture();
		break;
	}

	return DefWindowProcW(window, message, wparam, lparam);
}

BOOL ClientWindowInitialize(const HINSTANCE instance) {
	WNDCLASSEXW windowClass = {0};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = windowProcedure;
	windowClass.hInstance = instance;
	windowClass.lpszClassName = CLASS_CLIENT_WINDOW;

	if(!RegisterClassExW(&windowClass)) {
		KHWIN32_LAST_ERROR(L"RegisterClassExW");
		return FALSE;
	}

	return TRUE;
}

DWORD WINAPI ClientWindowThread(_In_ PCLIENT client) {
	if(!client) {
		LOG("[Window]: No client structure provided\n");
		return 1;
	}

	if(!client->window) {
		LOG("[Window]: No window structure provided\n");
		return 1;
	}

	LOG("[Window %ws]: Starting\n" COMMA client->address);
	memset(&client->window->stream, 0, sizeof(STREAMDATA));
	memset(&client->window->menu, 0, sizeof(WINDOWCONTEXTMENU));
	client->window->menu.method = SEND_METHOD_COLOR;
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int width = (int) (((double) screenWidth) * 0.439238653);
	int height = (int) (((double) screenHeight) * 0.520833333);
	LPWSTR windowName = KHFormatMessageW(L"%ws [%ws]", client->name, client->address);
	client->window->stream.resizeActivationDistance = (int) (((double) screenWidth) * 0.00878477306);
	client->window->window = CreateWindowExW(0L, CLASS_CLIENT_WINDOW, windowName ? windowName : L"Client Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, (screenWidth - width) / 2, (screenHeight - height) / 2, width, height, NULL, NULL, NULL, client);

	if(windowName) {
		LocalFree(windowName);
	}

	screenWidth = 1;

	if(!client->window->window) {
		KHWIN32_LAST_ERROR(L"CreateWindowExW");
		goto functionExit;
	}

	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	client->window->menu.stream = FALSE;
	sendFrameCode(client);

	if(WaitForSingleObject(client->window->lock, INFINITE) == WAIT_FAILED) {
		KHWIN32_LAST_ERROR(L"WaitForSingleObject");
		goto functionExit;
	}

	if(client->window->stream.pixels) {
		LocalFree(client->window->stream.pixels);
		client->window->stream.pixels = NULL;
	}

	screenWidth = 0;
functionExit:
	CloseHandle(client->window->lock);
	LOG("[Window %ws]: Exit client window with code: %d\n" COMMA client->address COMMA screenWidth);
	CloseHandle(client->window->thread);
	LocalFree(client->window);
	client->window = NULL;
	return screenWidth;
}

void ClientWindowExit(const PCLIENT client) {
	if(client->window) {
		PostMessageW(client->window->window, WM_CLOSE, 0, 0);
	}
}
