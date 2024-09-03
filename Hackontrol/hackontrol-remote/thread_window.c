#include "thread_window.h"
#include <khopanwin32.h>
#include <khopanstring.h>
#include <hackontrolpacket.h>
#include "logger.h"

#define CLASS_CLIENT_WINDOW L"HackontrolRemoteClientWindow"

#define IDM_PICTURE_IN_PICTURE              0xE001
#define IDM_LOCK_FRAME                      0xE002
#define IDM_LIMIT_TO_SCREEN                 0xE003
#define IDM_WINDOW_EXIT                     0xE004
#define IDM_WINDOW_STREAMING_ENABLE         0xE005
#define IDM_WINDOW_SEND_METHOD_FULL         0xE006
#define IDM_WINDOW_SEND_METHOD_BOUNDARY     0xE007
#define IDM_WINDOW_SEND_METHOD_COLOR        0xE008
#define IDM_WINDOW_SEND_METHOD_UNCOMPRESSED 0xE009

static void limitToScreen(const HWND window) {
	RECT bounds;
	GetWindowRect(window, &bounds);
	bounds.right -= bounds.left;
	bounds.bottom -= bounds.top;
	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);
	bounds.left = max(bounds.left, 0);
	bounds.top = max(bounds.top, 0);
	bounds.right = min(bounds.right, width);
	bounds.bottom = min(bounds.bottom, height);
	if(bounds.left + bounds.right > width) bounds.left = width - bounds.right;
	if(bounds.top + bounds.bottom > height) bounds.top = height - bounds.bottom;
	SetWindowPos(window, HWND_TOP, bounds.left, bounds.top, bounds.right, bounds.bottom, 0);
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
	case WM_ERASEBKGND:
		return 1;
	}

	RECT bounds;
	GetClientRect(window, &bounds);
	bounds.right -= bounds.left;
	bounds.bottom -= bounds.top;

	if(bounds.right < 1 || bounds.bottom < 1 || WaitForSingleObject(client->window->lock, INFINITE) == WAIT_FAILED) {
		return DefWindowProcW(window, message, wparam, lparam);
	}

	LRESULT returnValue = 0;
	POINT position;

	switch(message) {
	case WM_SIZE: {
		client->window->stream.imageWidth = (int) (((double) client->window->stream.originalImageWidth) / ((double) client->window->stream.originalImageHeight) * ((double) bounds.bottom));
		client->window->stream.imageHeight = (int) (((double) client->window->stream.originalImageHeight) / ((double) client->window->stream.originalImageWidth) * ((double) bounds.right));

		if(client->window->stream.imageWidth < bounds.right) {
			client->window->stream.imageHeight = bounds.bottom;
			client->window->stream.imageX = (int) ((((double) bounds.right) - ((double) client->window->stream.imageWidth)) / 2.0);
			client->window->stream.imageY = 0;
		} else {
			client->window->stream.imageWidth = bounds.right;
			client->window->stream.imageX = 0;
			client->window->stream.imageY = (int) ((((double) bounds.bottom) - ((double) client->window->stream.imageHeight)) / 2.0);
		}

		goto releaseMutex;
	}
	case WM_PAINT: {
		PAINTSTRUCT paintStruct;
		HDC context = BeginPaint(window, &paintStruct);
		HDC memoryContext = CreateCompatibleDC(context);
		HBITMAP bitmap = CreateCompatibleBitmap(context, bounds.right, bounds.bottom);
		HBITMAP oldBitmap = SelectObject(memoryContext, bitmap);
		HBRUSH brush = GetStockObject(DC_BRUSH);
		SetDCBrushColor(memoryContext, 0x000000);
		FillRect(memoryContext, &bounds, brush);

		if(client->window->stream.pixels) {
			SetStretchBltMode(memoryContext, HALFTONE);
			BITMAPINFO information = {0};
			information.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			information.bmiHeader.biWidth = client->window->stream.originalImageWidth;
			information.bmiHeader.biHeight = client->window->stream.originalImageHeight;
			information.bmiHeader.biPlanes = 1;
			information.bmiHeader.biBitCount = 32;
			StretchDIBits(memoryContext, client->window->stream.imageX, client->window->stream.imageY, client->window->stream.imageWidth, client->window->stream.imageHeight, 0, 0, client->window->stream.originalImageWidth, client->window->stream.originalImageHeight, client->window->stream.pixels, &information, DIB_RGB_COLORS, SRCCOPY);
		}

		BitBlt(context, 0, 0, bounds.right, bounds.bottom, memoryContext, 0, 0, SRCCOPY);
		SelectObject(memoryContext, oldBitmap);
		DeleteObject(bitmap);
		DeleteDC(memoryContext);
		EndPaint(window, &paintStruct);
		goto releaseMutex;
	}
	case WM_CONTEXTMENU: {
		SetForegroundWindow(window);
		TrackPopupMenuEx(client->window->contextMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, LOWORD(lparam), HIWORD(lparam), window, NULL);
		goto releaseMutex;
	}
	case WM_COMMAND: {
		switch(LOWORD(wparam)) {
		case IDM_PICTURE_IN_PICTURE:
			client->window->stream.pictureInPictureMode = !client->window->stream.pictureInPictureMode;
			CheckMenuItem(client->window->contextMenu, IDM_PICTURE_IN_PICTURE, MF_BYCOMMAND | (client->window->stream.pictureInPictureMode ? MF_CHECKED : MF_UNCHECKED));
			position.x = client->window->stream.pictureInPictureMode ? MF_ENABLED : MF_DISABLED;
			EnableMenuItem(client->window->contextMenu, IDM_LOCK_FRAME, position.x);
			EnableMenuItem(client->window->contextMenu, IDM_LIMIT_TO_SCREEN, position.x);
			SetWindowLongPtrW(window, GWL_STYLE, (client->window->stream.pictureInPictureMode ? WS_POPUP : WS_OVERLAPPEDWINDOW) | WS_VISIBLE);
			SetWindowPos(window, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
			PostMessageW(window, WM_SIZE, 0, 0);
			goto releaseMutex;
		case IDM_LOCK_FRAME:
			client->window->stream.lockFrame = !client->window->stream.lockFrame;
			CheckMenuItem(client->window->contextMenu, IDM_LOCK_FRAME, MF_BYCOMMAND | (client->window->stream.lockFrame ? MF_CHECKED : MF_UNCHECKED));
			goto releaseMutex;
		case IDM_LIMIT_TO_SCREEN:
			client->window->stream.limitToScreen = !client->window->stream.limitToScreen;
			CheckMenuItem(client->window->contextMenu, IDM_LIMIT_TO_SCREEN, MF_BYCOMMAND | (client->window->stream.limitToScreen ? MF_CHECKED : MF_UNCHECKED));
			if(client->window->stream.limitToScreen) limitToScreen(window);
			goto releaseMutex;
		case IDM_WINDOW_EXIT:
			LOG("[Window %ws]: Exiting\n" COMMA client->address);
			ClientDisconnect(client);
			goto releaseMutex;
		case IDM_WINDOW_STREAMING_ENABLE:
			client->window->stream.streaming = !client->window->stream.streaming;
			CheckMenuItem(client->window->streamingMenu, IDM_WINDOW_STREAMING_ENABLE, MF_BYCOMMAND | (client->window->stream.streaming ? MF_CHECKED : MF_UNCHECKED));
			goto sendStreamCode;
		case IDM_WINDOW_SEND_METHOD_FULL:
			client->window->stream.sendMethod = SEND_METHOD_FULL;
			CheckMenuItem(client->window->contextMenu, IDM_WINDOW_SEND_METHOD_FULL, MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(client->window->contextMenu, IDM_WINDOW_SEND_METHOD_BOUNDARY, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(client->window->contextMenu, IDM_WINDOW_SEND_METHOD_COLOR, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(client->window->contextMenu, IDM_WINDOW_SEND_METHOD_UNCOMPRESSED, MF_BYCOMMAND | MF_UNCHECKED);
			goto sendStreamCode;
		case IDM_WINDOW_SEND_METHOD_BOUNDARY:
			client->window->stream.sendMethod = SEND_METHOD_BOUNDARY;
			CheckMenuItem(client->window->contextMenu, IDM_WINDOW_SEND_METHOD_FULL, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(client->window->contextMenu, IDM_WINDOW_SEND_METHOD_BOUNDARY, MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(client->window->contextMenu, IDM_WINDOW_SEND_METHOD_COLOR, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(client->window->contextMenu, IDM_WINDOW_SEND_METHOD_UNCOMPRESSED, MF_BYCOMMAND | MF_UNCHECKED);
			goto sendStreamCode;
		case IDM_WINDOW_SEND_METHOD_COLOR:
			client->window->stream.sendMethod = SEND_METHOD_COLOR;
			CheckMenuItem(client->window->contextMenu, IDM_WINDOW_SEND_METHOD_FULL, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(client->window->contextMenu, IDM_WINDOW_SEND_METHOD_BOUNDARY, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(client->window->contextMenu, IDM_WINDOW_SEND_METHOD_COLOR, MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(client->window->contextMenu, IDM_WINDOW_SEND_METHOD_UNCOMPRESSED, MF_BYCOMMAND | MF_UNCHECKED);
			goto sendStreamCode;
		case IDM_WINDOW_SEND_METHOD_UNCOMPRESSED:
			client->window->stream.sendMethod = SEND_METHOD_UNCOMPRESSED;
			CheckMenuItem(client->window->contextMenu, IDM_WINDOW_SEND_METHOD_FULL, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(client->window->contextMenu, IDM_WINDOW_SEND_METHOD_BOUNDARY, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(client->window->contextMenu, IDM_WINDOW_SEND_METHOD_COLOR, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(client->window->contextMenu, IDM_WINDOW_SEND_METHOD_UNCOMPRESSED, MF_BYCOMMAND | MF_CHECKED);
sendStreamCode:
			position.x = (BYTE) ((client->window->stream.sendMethod & 0b11) << 1) | (client->window->stream.streaming ? 0b1001 : 0);
			PACKET packet;
			packet.size = 1;
			packet.packetType = PACKET_TYPE_STREAM_FRAME;
			packet.data = &position.x;
			SendPacket(client->socket, &packet);
			goto releaseMutex;
		}
	}
	case WM_MOUSEMOVE: {
		if(!client->window->stream.pictureInPictureMode || client->window->stream.lockFrame) {
			SetCursor(LoadCursorW(NULL, IDC_ARROW));
			break;
		}

		if(wparam != MK_LBUTTON) {
			position.x = LOWORD(lparam);
			position.y = HIWORD(lparam);
			client->window->stream.cursorNorth = position.y >= 0 && position.y <= client->window->stream.resizeActivationDistance;
			client->window->stream.cursorEast = position.x >= bounds.right - client->window->stream.resizeActivationDistance && position.x < bounds.right;
			client->window->stream.cursorSouth = position.y >= bounds.bottom - client->window->stream.resizeActivationDistance && position.y < bounds.bottom;
			client->window->stream.cursorWest = position.x >= 0 && position.x <= client->window->stream.resizeActivationDistance;
			SetCursor(LoadCursorW(NULL, client->window->stream.cursorNorth ? client->window->stream.cursorWest ? IDC_SIZENWSE : client->window->stream.cursorEast ? IDC_SIZENESW : IDC_SIZENS : client->window->stream.cursorSouth ? client->window->stream.cursorWest ? IDC_SIZENESW : client->window->stream.cursorEast ? IDC_SIZENWSE : IDC_SIZENS : client->window->stream.cursorWest ? IDC_SIZEWE : client->window->stream.cursorEast ? IDC_SIZEWE : IDC_ARROW));
			break;
		}

		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);
		GetCursorPos(&position);

		if(!client->window->stream.cursorNorth && !client->window->stream.cursorEast && !client->window->stream.cursorSouth && !client->window->stream.cursorWest) {
			position.x -= client->window->stream.position.x - client->window->stream.bounds.left;
			position.y -= client->window->stream.position.y - client->window->stream.bounds.top;
			SetWindowPos(window, HWND_TOP, client->window->stream.limitToScreen ? position.x < 0 ? 0 : position.x + client->window->stream.bounds.right - client->window->stream.bounds.left > screenWidth ? screenWidth - client->window->stream.bounds.right + client->window->stream.bounds.left : position.x : position.x, client->window->stream.limitToScreen ? position.y < 0 ? 0 : position.y + client->window->stream.bounds.bottom - client->window->stream.bounds.top > screenHeight ? screenHeight - client->window->stream.bounds.bottom + client->window->stream.bounds.top : position.y : position.y, 0, 0, SWP_NOSIZE);
			break;
		}

		int minimumSize = client->window->stream.resizeActivationDistance * 3;
		int difference;
		GetWindowRect(window, &bounds);
		bounds.right -= bounds.left;
		bounds.bottom -= bounds.top;

		if(client->window->stream.cursorNorth) {
			difference = position.y - client->window->stream.position.y;
			if(client->window->stream.limitToScreen && client->window->stream.bounds.top + difference < 0) difference = -client->window->stream.bounds.top;
			if(client->window->stream.bounds.bottom - client->window->stream.bounds.top - difference < minimumSize) difference = client->window->stream.bounds.bottom - client->window->stream.bounds.top - minimumSize;
			bounds.top = client->window->stream.bounds.top + difference;
			bounds.bottom = client->window->stream.bounds.bottom - bounds.top;
		}

		if(client->window->stream.cursorEast) {
			difference = position.x - client->window->stream.position.x;
			if(client->window->stream.limitToScreen && client->window->stream.bounds.right + difference > screenWidth) difference = screenWidth - client->window->stream.bounds.right;
			bounds.right = client->window->stream.bounds.right - client->window->stream.bounds.left + difference;
			bounds.right = max(bounds.right, minimumSize);
		}

		if(client->window->stream.cursorSouth) {
			difference = position.y - client->window->stream.position.y;
			if(client->window->stream.limitToScreen && client->window->stream.bounds.bottom + difference > screenHeight) difference = screenHeight - client->window->stream.bounds.bottom;
			bounds.bottom = client->window->stream.bounds.bottom - client->window->stream.bounds.top + difference;
			bounds.bottom = max(bounds.bottom, minimumSize);
		}

		if(client->window->stream.cursorWest) {
			difference = position.x - client->window->stream.position.x;
			if(client->window->stream.limitToScreen && client->window->stream.bounds.left + difference < 0) difference = -client->window->stream.bounds.left;
			if(client->window->stream.bounds.right - client->window->stream.bounds.left - difference < minimumSize) difference = client->window->stream.bounds.right - client->window->stream.bounds.left - minimumSize;
			bounds.left = client->window->stream.bounds.left + difference;
			bounds.right = client->window->stream.bounds.right - bounds.left;
		}

		SetWindowPos(window, HWND_TOP, bounds.left, bounds.top, bounds.right, bounds.bottom, (!client->window->stream.cursorNorth && client->window->stream.cursorEast) || (client->window->stream.cursorSouth && !client->window->stream.cursorWest) ? SWP_NOMOVE : 0);
		break;
	}
	case WM_LBUTTONDOWN:
		GetWindowRect(window, &client->window->stream.bounds);
		GetCursorPos(&client->window->stream.position);
		SetCapture(window);
		break;
	case WM_LBUTTONUP:
		ReleaseCapture();
		break;
	}

	ReleaseMutex(client->window->lock);
	returnValue = DefWindowProcW(window, message, wparam, lparam);
	goto functionExit;
releaseMutex:
	ReleaseMutex(client->window->lock);
functionExit:
	return returnValue;
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
	client->window->stream.sendMethod = SEND_METHOD_COLOR;
	client->window->contextMenu = CreatePopupMenu();

	if(!client->window->contextMenu) {
		KHWIN32_LAST_ERROR(L"CreatePopupMenu");
		goto functionExit;
	}

	client->window->streamingMenu = CreateMenu();

	if(!client->window->streamingMenu) {
		KHWIN32_LAST_ERROR(L"CreateMenu");
		goto destroyContextMenu;
	}

	client->window->sendMethodMenu = CreateMenu();

	if(!client->window->sendMethodMenu) {
		KHWIN32_LAST_ERROR(L"CreateMenu");
		goto destroyStreamingMenu;
	}

	AppendMenuW(client->window->streamingMenu, MF_STRING, IDM_WINDOW_STREAMING_ENABLE, L"Enable");
	AppendMenuW(client->window->sendMethodMenu, MF_STRING, IDM_WINDOW_SEND_METHOD_FULL, L"Full");
	AppendMenuW(client->window->sendMethodMenu, MF_STRING, IDM_WINDOW_SEND_METHOD_BOUNDARY, L"Boundary Differences");
	AppendMenuW(client->window->sendMethodMenu, MF_STRING, IDM_WINDOW_SEND_METHOD_COLOR, L"Color Differences");
	AppendMenuW(client->window->sendMethodMenu, MF_STRING, IDM_WINDOW_SEND_METHOD_UNCOMPRESSED, L"Uncompressed");
	AppendMenuW(client->window->streamingMenu, MF_POPUP | MF_DISABLED, (UINT_PTR) client->window->sendMethodMenu, L"Send Method");
	AppendMenuW(client->window->contextMenu, MF_POPUP, (UINT_PTR) client->window->streamingMenu, L"Streaming");
	AppendMenuW(client->window->contextMenu, MF_STRING, IDM_PICTURE_IN_PICTURE, L"Picture in Picture Mode");
	AppendMenuW(client->window->contextMenu, MF_STRING | MF_DISABLED, IDM_LOCK_FRAME, L"Lock Frame");
	AppendMenuW(client->window->contextMenu, MF_STRING | MF_DISABLED, IDM_LIMIT_TO_SCREEN, L"Limit to Screen");
	AppendMenuW(client->window->contextMenu, MF_STRING, IDM_WINDOW_EXIT, L"Exit");
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

	client->window->stream.streaming = FALSE;
	message.message = 0;
	PACKET packet;
	packet.size = 1;
	packet.packetType = PACKET_TYPE_STREAM_FRAME;
	packet.data = &message.message;
	SendPacket(client->socket, &packet);

	if(WaitForSingleObject(client->window->lock, INFINITE) == WAIT_FAILED) {
		KHWIN32_LAST_ERROR(L"WaitForSingleObject");
		goto destroySendMethodMenu;
	}

	if(client->window->stream.pixels) {
		LocalFree(client->window->stream.pixels);
		client->window->stream.pixels = NULL;
	}

	screenWidth = 0;
destroySendMethodMenu:
	DestroyMenu(client->window->sendMethodMenu);
destroyStreamingMenu:
	DestroyMenu(client->window->streamingMenu);
destroyContextMenu:
	DestroyMenu(client->window->contextMenu);
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
