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

static void streamCodeSend(const PCLIENT client) {
	BYTE flags = ((client->window->stream.sendMethod & 0b11) << 1) | (client->window->stream.streaming ? 0b1001 : 0);
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
		AppendMenuW(popupMenu, MF_STRING | (client->window->stream.lockFrame ? MF_CHECKED : MF_UNCHECKED) | (client->window->stream.pictureInPictureMode ? MF_ENABLED : MF_DISABLED), IDM_LOCK_FRAME, L"Lock Frame");
		AppendMenuW(popupMenu, MF_STRING | (client->window->stream.limitToScreen ? MF_CHECKED : MF_UNCHECKED) | (client->window->stream.pictureInPictureMode ? MF_ENABLED : MF_DISABLED), IDM_LIMIT_TO_SCREEN, L"Limit to Screen");
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
			SetWindowLongPtrW(window, GWL_STYLE, (client->window->stream.pictureInPictureMode ? WS_POPUP : WS_OVERLAPPEDWINDOW) | WS_VISIBLE);
			SetWindowPos(window, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
			PostMessageW(window, WM_SIZE, 0, 0);
			break;
		case IDM_LOCK_FRAME:
			client->window->stream.lockFrame = !client->window->stream.lockFrame;
			break;
		case IDM_LIMIT_TO_SCREEN:
			client->window->stream.limitToScreen = !client->window->stream.limitToScreen;
			break;
		case IDM_WINDOW_EXIT:
			LOG("[Window %ws]: Exiting\n" COMMA client->address);
			ClientDisconnect(client);
			break;
		case IDM_WINDOW_STREAMING_ENABLE:
			client->window->stream.streaming = !client->window->stream.streaming;
			streamCodeSend(client);
			break;
		case IDM_WINDOW_SEND_METHOD_FULL:
		case IDM_WINDOW_SEND_METHOD_BOUNDARY:
		case IDM_WINDOW_SEND_METHOD_COLOR:
		case IDM_WINDOW_SEND_METHOD_UNCOMPRESSED:
			client->window->stream.sendMethod = position.x == IDM_WINDOW_SEND_METHOD_FULL ? SEND_METHOD_FULL : position.x == IDM_WINDOW_SEND_METHOD_BOUNDARY ? SEND_METHOD_BOUNDARY : position.x == IDM_WINDOW_SEND_METHOD_COLOR ? SEND_METHOD_COLOR : SEND_METHOD_UNCOMPRESSED;
			streamCodeSend(client);
			break;
		}

		goto releaseMutex;
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

		GetWindowRect(window, &bounds);

		/*if(position.x + client->window->stream.pressedOffsetX < GetSystemMetrics(SM_CXSCREEN)) {
			bounds.left = position.x - client->window->stream.pressedX;
			bounds.right = position.x + client->window->stream.pressedOffsetX;
		}

		if(position.y + client->window->stream.pressedOffsetY < GetSystemMetrics(SM_CYSCREEN)) {
			bounds.top = position.y - client->window->stream.pressedY;
			bounds.bottom = position.y + client->window->stream.pressedOffsetY;
		}*/

		/*if(client->window->stream.cursorNorth) {
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
		}*/

		//bounds.right -= bounds.left;
		//bounds.bottom -= bounds.top;

		//if(client->window->stream.limitToScreen) {
			/*bounds.left = max(bounds.left, 0);
			bounds.top = max(bounds.top, 0);
			bounds.right = min(bounds.right, GetSystemMetrics(SM_CXSCREEN));
			bounds.bottom = min(bounds.bottom, GetSystemMetrics(SM_CYSCREEN));*/
			//limitToScreen(&bounds);
		//}

		//LOG("Right: %d Bottom %d\n" COMMA bounds.right COMMA bounds.bottom);
		bounds.right -= bounds.left;
		bounds.bottom -= bounds.top;
		int minimumSize = client->window->stream.resizeActivationDistance * 3;
		int difference;

		if(client->window->stream.cursorNorth) {
			difference = position.y - client->window->stream.mouseY;

			if(client->window->stream.savedTop + difference < 0) {
				difference = -client->window->stream.savedTop;
			}

			if(client->window->stream.savedBottom - client->window->stream.savedTop - difference < minimumSize) {
				difference = client->window->stream.savedBottom - client->window->stream.savedTop - minimumSize;
			}

			bounds.top = client->window->stream.savedTop + difference;
			bounds.bottom = client->window->stream.savedBottom - bounds.top;
		}

		/*if(client->window->stream.cursorEast) {
			bounds.right = position.x + client->window->stream.pressedOffsetX - bounds.left;

			if(bounds.left + bounds.right > maxWidth) {
				bounds.right = maxWidth - bounds.left;
			}

			if(bounds.right < minimumSize) {
				bounds.right = minimumSize;
			}
		}*/

		if(client->window->stream.cursorSouth) {
			difference = position.y - client->window->stream.mouseY;

			if(client->window->stream.savedBottom + difference > screenHeight) {
				difference = -client->window->stream.savedBottom + screenHeight;
			}

			bounds.bottom = client->window->stream.savedBottom - client->window->stream.savedTop + difference;

			if(bounds.bottom < minimumSize) {
				bounds.bottom = minimumSize;
			}
		}

		SetWindowPos(window, HWND_TOP, bounds.left, bounds.top, bounds.right, bounds.bottom, 0);
		break;
	}
	case WM_LBUTTONDOWN:
		GetWindowRect(window, &bounds);
		GetCursorPos(&position);
		client->window->stream.pressedX = position.x - bounds.left;
		client->window->stream.pressedY = position.y - bounds.top;
		client->window->stream.pressedOffsetX = bounds.right - position.x;
		client->window->stream.pressedOffsetY = bounds.bottom - position.y;
		client->window->stream.mouseX = position.x;
		client->window->stream.mouseY = position.y;
		client->window->stream.savedTop = bounds.top;
		client->window->stream.savedBottom = bounds.bottom;
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
	streamCodeSend(client);

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
