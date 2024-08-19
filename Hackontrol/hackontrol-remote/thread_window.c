#include "thread_window.h"
#include <khopanwin32.h>
#include <khopanstring.h>
#include <hackontrolpacket.h>
#include "logger.h"

#define IDM_PICTURE_IN_PICTURE              0xE001
#define IDM_WINDOW_EXIT                     0xE002
#define IDM_WINDOW_STREAMING_ENABLE         0xE003
#define IDM_WINDOW_SEND_METHOD_FULL         0xE004
#define IDM_WINDOW_SEND_METHOD_BOUNDARY     0xE005
#define IDM_WINDOW_SEND_METHOD_COLOR        0xE006
#define IDM_WINDOW_SEND_METHOD_UNCOMPRESSED 0xE007

extern HINSTANCE programInstance;

static void sendStreamCode(const PCLIENT client) {
	unsigned char flags = ((client->stream->sendMethod & 0b11) << 1) | (client->stream->streaming ? 0b1001 : 0);
	LOG("[Window Thread %ws]: Flags: %c%c%c%c%c%c%c%c\n" COMMA client->address COMMA flags & 0x80 ? '1' : '0' COMMA flags & 0x40 ? '1' : '0' COMMA flags & 0x20 ? '1' : '0' COMMA flags & 0x10 ? '1' : '0' COMMA flags & 0x08 ? '1' : '0' COMMA flags & 0x04 ? '1' : '0' COMMA flags & 0x02 ? '1' : '0' COMMA flags & 0x01 ? '1' : '0');
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

	if(bounds.right < 1 || bounds.bottom < 1) {
		return DefWindowProcW(window, message, wparam, lparam);
	}

	WaitForSingleObject(client->stream->lock, INFINITE);
	LRESULT returnValue = 0;
	POINT position;

	switch(message) {
	case WM_SIZE: {
		client->stream->imageWidth = (int) (((double) client->stream->originalImageWidth) / ((double) client->stream->originalImageHeight) * ((double) bounds.bottom));
		client->stream->imageHeight = (int) (((double) client->stream->originalImageHeight) / ((double) client->stream->originalImageWidth) * ((double) bounds.right));

		if(client->stream->imageWidth < bounds.right) {
			client->stream->imageHeight = bounds.bottom;
			client->stream->imageX = (int) ((((double) bounds.right) - ((double) client->stream->imageWidth)) / 2.0);
			client->stream->imageY = 0;
		} else {
			client->stream->imageWidth = bounds.right;
			client->stream->imageX = 0;
			client->stream->imageY = (int) ((((double) bounds.bottom) - ((double) client->stream->imageHeight)) / 2.0);
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

		if(client->stream->pixels) {
			SetStretchBltMode(memoryContext, HALFTONE);
			BITMAPINFO information = {0};
			information.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			information.bmiHeader.biWidth = client->stream->originalImageWidth;
			information.bmiHeader.biHeight = client->stream->originalImageHeight;
			information.bmiHeader.biPlanes = 1;
			information.bmiHeader.biBitCount = 32;
			StretchDIBits(memoryContext, client->stream->imageX, client->stream->imageY, client->stream->imageWidth, client->stream->imageHeight, 0, 0, client->stream->originalImageWidth, client->stream->originalImageHeight, client->stream->pixels, &information, DIB_RGB_COLORS, SRCCOPY);
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

		AppendMenuW(streamingMenu, MF_STRING | (client->stream->streaming ? MF_CHECKED : MF_UNCHECKED), IDM_WINDOW_STREAMING_ENABLE, L"Enable");
		AppendMenuW(sendMethod, MF_STRING | (client->stream->sendMethod == SEND_METHOD_FULL ? MF_CHECKED : MF_UNCHECKED), IDM_WINDOW_SEND_METHOD_FULL, L"Full");
		AppendMenuW(sendMethod, MF_STRING | (client->stream->sendMethod == SEND_METHOD_BOUNDARY ? MF_CHECKED : MF_UNCHECKED), IDM_WINDOW_SEND_METHOD_BOUNDARY, L"Boundary Differences");
		AppendMenuW(sendMethod, MF_STRING | (client->stream->sendMethod == SEND_METHOD_COLOR ? MF_CHECKED : MF_UNCHECKED), IDM_WINDOW_SEND_METHOD_COLOR, L"Color Differences");
		AppendMenuW(sendMethod, MF_STRING | (client->stream->sendMethod == SEND_METHOD_UNCOMPRESSED ? MF_CHECKED : MF_UNCHECKED), IDM_WINDOW_SEND_METHOD_UNCOMPRESSED, L"Uncompressed");
		AppendMenuW(streamingMenu, MF_POPUP | (client->stream->streaming ? MF_ENABLED : MF_DISABLED), (UINT_PTR) sendMethod, L"Send Method");
		AppendMenuW(popupMenu, MF_POPUP, (UINT_PTR) streamingMenu, L"Streaming");
		AppendMenuW(popupMenu, MF_STRING | (client->stream->pictureInPictureMode ? MF_CHECKED : MF_UNCHECKED), IDM_PICTURE_IN_PICTURE, L"Picture in Picture Mode");
		AppendMenuW(popupMenu, MF_STRING, IDM_WINDOW_EXIT, L"Exit");
		SetForegroundWindow(window);
		ReleaseMutex(client->stream->lock);
		position.x = TrackPopupMenuEx(popupMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, LOWORD(lparam), HIWORD(lparam), window, NULL);
		DestroyMenu(sendMethod);
		DestroyMenu(streamingMenu);
		DestroyMenu(popupMenu);
		WaitForSingleObject(client->stream->lock, INFINITE);

		switch(position.x) {
		case IDM_PICTURE_IN_PICTURE:
			client->stream->pictureInPictureMode = !client->stream->pictureInPictureMode;
			SetWindowLongPtrW(client->clientWindow, GWL_STYLE, (client->stream->pictureInPictureMode ? WS_POPUP : WS_OVERLAPPEDWINDOW) | WS_VISIBLE);
			PostMessageW(window, WM_SIZE, 0, MAKELONG(bounds.right, bounds.bottom));
			break;
		case IDM_WINDOW_EXIT:
			LOG("[Window Thread %ws]: Exiting\n" COMMA client->address);
			ClientDisconnect(client);
			break;
		case IDM_WINDOW_STREAMING_ENABLE:
			client->stream->streaming = !client->stream->streaming;
			sendStreamCode(client);
			break;
		case IDM_WINDOW_SEND_METHOD_FULL:
		case IDM_WINDOW_SEND_METHOD_BOUNDARY:
		case IDM_WINDOW_SEND_METHOD_COLOR:
		case IDM_WINDOW_SEND_METHOD_UNCOMPRESSED:
			client->stream->sendMethod = position.x == IDM_WINDOW_SEND_METHOD_FULL ? SEND_METHOD_FULL : position.x == IDM_WINDOW_SEND_METHOD_BOUNDARY ? SEND_METHOD_BOUNDARY : position.x == IDM_WINDOW_SEND_METHOD_COLOR ? SEND_METHOD_COLOR : SEND_METHOD_UNCOMPRESSED;
			sendStreamCode(client);
			break;
		}

		goto releaseMutex;
	}
	case WM_MOUSEMOVE: {
		if(wparam != MK_LBUTTON) {
		#define BORDER 10
			position.x = LOWORD(lparam);
			position.y = HIWORD(lparam);
			client->stream->cursorNorth = position.y >= 0 && position.y <= BORDER;
			client->stream->cursorEast = position.x >= bounds.right - BORDER && position.x < bounds.right;
			client->stream->cursorSouth = position.y >= bounds.bottom - BORDER && position.y < bounds.bottom;
			client->stream->cursorWest = position.x >= 0 && position.x <= BORDER;
			SetCursor(LoadCursorW(NULL, client->stream->cursorNorth ? client->stream->cursorWest ? IDC_SIZENWSE : client->stream->cursorEast ? IDC_SIZENESW : IDC_SIZENS : client->stream->cursorSouth ? client->stream->cursorWest ? IDC_SIZENESW : client->stream->cursorEast ? IDC_SIZENWSE : IDC_SIZENS : client->stream->cursorWest ? IDC_SIZEWE : client->stream->cursorEast ? IDC_SIZEWE : IDC_ARROW));
			break;
		}

		GetCursorPos(&position);

		if(!client->stream->cursorNorth && !client->stream->cursorEast && !client->stream->cursorSouth && !client->stream->cursorWest) {
			SetWindowPos(window, HWND_TOP, position.x - client->stream->pressedX, position.y - client->stream->pressedY, 0, 0, SWP_NOSIZE);
			break;
		}

		if(client->stream->cursorEast) {
			GetWindowRect(window, &bounds);
			SetWindowPos(window, HWND_TOP, 0, 0, position.x - bounds.left + client->stream->pressedOffsetX, bounds.bottom - bounds.top, SWP_NOMOVE);
			break;
		}

		if(client->stream->cursorSouth) {
			GetWindowRect(window, &bounds);
			SetWindowPos(window, HWND_TOP, 0, 0, bounds.right - bounds.left, position.y - bounds.top + client->stream->pressedOffsetY, SWP_NOMOVE);
			break;
		}

		SetWindowPos(window, HWND_TOP, position.x, position.y, 0, 0, SWP_NOSIZE);
		break;
	}
	case WM_LBUTTONDOWN:
		client->stream->pressedX = LOWORD(lparam);
		client->stream->pressedY = HIWORD(lparam);
		client->stream->pressedOffsetX = bounds.right - client->stream->pressedX;
		client->stream->pressedOffsetY = bounds.bottom - client->stream->pressedY;
		SetCapture(window);
		break;
	case WM_LBUTTONUP:
		ReleaseCapture();
		break;
	}

	returnValue = DefWindowProcW(window, message, wparam, lparam);
releaseMutex:
	ReleaseMutex(client->stream->lock);
	return returnValue;
}

BOOL WindowRegisterClass() {
	WNDCLASSEXW windowClass = {0};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = windowProcedure;
	windowClass.hInstance = programInstance;
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

	if(client->stream) {
		LocalFree(client->stream);
	}

	client->stream = LocalAlloc(LMEM_FIXED, sizeof(STREAMDATA));
	int returnValue = 1;

	if(!client->stream) {
		KHWin32DialogErrorW(GetLastError(), L"LocalAlloc");
		goto exit;
	}

	memset(client->stream, 0, sizeof(STREAMDATA));
	client->stream->lock = CreateMutexExW(NULL, NULL, 0, DELETE | SYNCHRONIZE);

	if(!client->stream->lock) {
		KHWin32DialogErrorW(GetLastError(), L"CreateMutexExW");
		goto freeStream;
	}

	client->stream->sendMethod = SEND_METHOD_COLOR;
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int width = (int) (((double) screenWidth) * 0.439238653);
	int height = (int) (((double) screenHeight) * 0.520833333);
	LPWSTR windowName = KHFormatMessageW(L"%ws [%ws]", client->name, client->address);
	client->clientWindow = CreateWindowExW(0L, CLASS_CLIENT_WINDOW, windowName ? windowName : L"Client Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, (screenWidth - width) / 2, (screenHeight - height) / 2, width, height, NULL, NULL, NULL, client);

	if(windowName) {
		LocalFree(windowName);
	}

	if(!client->clientWindow) {
		KHWin32DialogErrorW(GetLastError(), L"CreateWindowExW");
		goto closeMutex;
	}

	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	WaitForSingleObject(client->stream->lock, INFINITE);
	client->stream->streaming = FALSE;
	sendStreamCode(client);

	if(client->stream->pixels) {
		LocalFree(client->stream->pixels);
	}

	client->clientWindow = NULL;
	returnValue = 0;
closeMutex:
	CloseHandle(client->stream->lock);
freeStream:
	if(client->stream) {
		LocalFree(client->stream);
		client->stream = NULL;
	}
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
