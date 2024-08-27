#include "thread_window.h"
#include <khopanwin32.h>
#include <khopanstring.h>
#include <hackontrolpacket.h>
#include "logger.h"

#define CLASS_CLIENT_WINDOW L"HackontrolRemoteClientWindow"

#define IDM_PICTURE_IN_PICTURE              0xE001
#define IDM_WINDOW_EXIT                     0xE002
#define IDM_WINDOW_STREAMING_ENABLE         0xE003
#define IDM_WINDOW_SEND_METHOD_FULL         0xE004
#define IDM_WINDOW_SEND_METHOD_BOUNDARY     0xE005
#define IDM_WINDOW_SEND_METHOD_COLOR        0xE006
#define IDM_WINDOW_SEND_METHOD_UNCOMPRESSED 0xE007

static void sendStreamCode(const PCLIENT client) {
	BYTE flags = ((client->window->stream.sendMethod & 0b11) << 1) | (client->window->stream.streaming ? 0b1001 : 0);
	LOG("[Window %ws]: Flags: %c%c%c%c%c%c%c%c\n" COMMA client->address COMMA flags & 0x80 ? '1' : '0' COMMA flags & 0x40 ? '1' : '0' COMMA flags & 0x20 ? '1' : '0' COMMA flags & 0x10 ? '1' : '0' COMMA flags & 0x08 ? '1' : '0' COMMA flags & 0x04 ? '1' : '0' COMMA flags & 0x02 ? '1' : '0' COMMA flags & 0x01 ? '1' : '0');
	PACKET packet;
	packet.size = 1;
	packet.packetType = PACKET_TYPE_STREAM_FRAME;
	packet.data = &flags;
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
		HMENU popupMenu = CreatePopupMenu();

		if(!popupMenu) {
			goto releaseMutex;
		}

		HMENU streamingMenu = CreateMenu();

		if(!streamingMenu) {
			DestroyMenu(popupMenu);
			goto releaseMutex;
		}

		HMENU sendMethod = CreateMenu();

		if(!sendMethod) {
			DestroyMenu(streamingMenu);
			DestroyMenu(popupMenu);
			goto releaseMutex;
		}

		AppendMenuW(streamingMenu, MF_STRING | (client->window->stream.streaming ? MF_CHECKED : MF_UNCHECKED), IDM_WINDOW_STREAMING_ENABLE, L"Enable");
		AppendMenuW(sendMethod, MF_STRING | (client->window->stream.sendMethod == SEND_METHOD_FULL ? MF_CHECKED : MF_UNCHECKED), IDM_WINDOW_SEND_METHOD_FULL, L"Full");
		AppendMenuW(sendMethod, MF_STRING | (client->window->stream.sendMethod == SEND_METHOD_BOUNDARY ? MF_CHECKED : MF_UNCHECKED), IDM_WINDOW_SEND_METHOD_BOUNDARY, L"Boundary Differences");
		AppendMenuW(sendMethod, MF_STRING | (client->window->stream.sendMethod == SEND_METHOD_COLOR ? MF_CHECKED : MF_UNCHECKED), IDM_WINDOW_SEND_METHOD_COLOR, L"Color Differences");
		AppendMenuW(sendMethod, MF_STRING | (client->window->stream.sendMethod == SEND_METHOD_UNCOMPRESSED ? MF_CHECKED : MF_UNCHECKED), IDM_WINDOW_SEND_METHOD_UNCOMPRESSED, L"Uncompressed");
		AppendMenuW(streamingMenu, MF_POPUP | (client->window->stream.streaming ? MF_ENABLED : MF_DISABLED), (UINT_PTR) sendMethod, L"Send Method");
		AppendMenuW(popupMenu, MF_POPUP, (UINT_PTR) streamingMenu, L"Streaming");
		AppendMenuW(popupMenu, MF_STRING | (client->window->stream.pictureInPictureMode ? MF_CHECKED : MF_UNCHECKED), IDM_PICTURE_IN_PICTURE, L"Picture in Picture Mode");
		AppendMenuW(popupMenu, MF_STRING, IDM_WINDOW_EXIT, L"Exit");
		SetForegroundWindow(window);
		ReleaseMutex(client->window->lock);
		position.x = TrackPopupMenuEx(popupMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, LOWORD(lparam), HIWORD(lparam), window, NULL);
		DestroyMenu(sendMethod);
		DestroyMenu(streamingMenu);
		DestroyMenu(popupMenu);

		if(WaitForSingleObject(client->window->lock, INFINITE) == WAIT_FAILED) {
			return 0;
		}

		switch(position.x) {
		case IDM_PICTURE_IN_PICTURE:
			client->window->stream.pictureInPictureMode = !client->window->stream.pictureInPictureMode;
			SetWindowLongPtrW(client->window->window, GWL_STYLE, (client->window->stream.pictureInPictureMode ? WS_POPUP : WS_OVERLAPPEDWINDOW) | WS_VISIBLE);
			PostMessageW(window, WM_SIZE, 0, MAKELONG(bounds.right, bounds.bottom));
			break;
		case IDM_WINDOW_EXIT:
			LOG("[Window %ws]: Exiting\n" COMMA client->address);
			ClientDisconnect(client);
			break;
		case IDM_WINDOW_STREAMING_ENABLE:
			client->window->stream.streaming = !client->window->stream.streaming;
			sendStreamCode(client);
			break;
		case IDM_WINDOW_SEND_METHOD_FULL:
		case IDM_WINDOW_SEND_METHOD_BOUNDARY:
		case IDM_WINDOW_SEND_METHOD_COLOR:
		case IDM_WINDOW_SEND_METHOD_UNCOMPRESSED:
			client->window->stream.sendMethod = position.x == IDM_WINDOW_SEND_METHOD_FULL ? SEND_METHOD_FULL : position.x == IDM_WINDOW_SEND_METHOD_BOUNDARY ? SEND_METHOD_BOUNDARY : position.x == IDM_WINDOW_SEND_METHOD_COLOR ? SEND_METHOD_COLOR : SEND_METHOD_UNCOMPRESSED;
			sendStreamCode(client);
			break;
		}

		goto releaseMutex;
	}
	case WM_MOUSEMOVE: {
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

		if(!client->window->stream.cursorNorth && !client->window->stream.cursorEast && !client->window->stream.cursorSouth && !client->window->stream.cursorWest) {
			client->window->stream.cursorNorth = TRUE;
			client->window->stream.cursorEast = TRUE;
			client->window->stream.cursorSouth = TRUE;
			client->window->stream.cursorWest = TRUE;
		}

		GetCursorPos(&position);
		GetWindowRect(window, &bounds);

		if(client->window->stream.cursorNorth) {
			bounds.top = position.y - client->window->stream.pressedY;
		}

		if(client->window->stream.cursorEast) {
			bounds.right = position.x + client->window->stream.pressedOffsetX;
		}

		if(client->window->stream.cursorSouth) {
			bounds.bottom = position.y + client->window->stream.pressedOffsetY;
		}

		if(client->window->stream.cursorWest) {
			bounds.left = position.x - client->window->stream.pressedX;
		}

		SetWindowPos(window, HWND_TOP, bounds.left, bounds.top, bounds.right - bounds.left, bounds.bottom - bounds.top, client->window->stream.cursorNorth && client->window->stream.cursorEast && client->window->stream.cursorSouth && client->window->stream.cursorWest ? SWP_NOSIZE : (!client->window->stream.cursorNorth && client->window->stream.cursorEast) || (client->window->stream.cursorSouth && !client->window->stream.cursorWest) ? SWP_NOMOVE : 0);
		break;
	}
	case WM_LBUTTONDOWN:
		client->window->stream.pressedX = LOWORD(lparam);
		client->window->stream.pressedY = HIWORD(lparam);
		client->window->stream.pressedOffsetX = bounds.right - client->window->stream.pressedX;
		client->window->stream.pressedOffsetY = bounds.bottom - client->window->stream.pressedY;
		SetCapture(window);
		break;
	case WM_LBUTTONUP:
		ReleaseCapture();
		break;
	}

	returnValue = DefWindowProcW(window, message, wparam, lparam);
releaseMutex:
	ReleaseMutex(client->window->lock);
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
		KHWin32DialogErrorW(GetLastError(), L"RegisterClassExW");
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
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	client->window->stream.resizeActivationDistance = (int) (((double) screenWidth) * 0.00878477306);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int width = (int) (((double) screenWidth) * 0.439238653);
	int height = (int) (((double) screenHeight) * 0.520833333);
	LPWSTR windowName = KHFormatMessageW(L"%ws [%ws]", client->name, client->address);
	client->window->window = CreateWindowExW(0L, CLASS_CLIENT_WINDOW, windowName ? windowName : L"Client Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, (screenWidth - width) / 2, (screenHeight - height) / 2, width, height, NULL, NULL, NULL, client);

	if(windowName) {
		LocalFree(windowName);
	}

	int returnValue = 1;

	if(!client->window->window) {
		KHWin32DialogErrorW(GetLastError(), L"CreateWindowExW");
		goto exit;
	}

	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	if(WaitForSingleObject(client->window->lock, INFINITE) == WAIT_FAILED) {
		goto exit;
	}

	client->window->stream.streaming = FALSE;
	sendStreamCode(client);

	if(client->window->stream.pixels) {
		LocalFree(client->window->stream.pixels);
	}

	returnValue = 0;
exit:
	CloseHandle(client->window->lock);
	LOG("[Window %ws]: Exit client window with code: %d\n" COMMA client->address COMMA returnValue);
	CloseHandle(client->window->thread);
	LocalFree(client->window);
	client->window = NULL;
	return returnValue;
}

void ClientWindowExit(const PCLIENT client) {
	if(client->window) {
		PostMessageW(client->window->window, WM_CLOSE, 0, 0);
	}
}
