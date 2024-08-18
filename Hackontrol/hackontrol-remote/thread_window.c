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
	unsigned char flags = ((client->stream->method & 0b11) << 1) | (client->stream->streaming ? 0b1001 : 0);
	LOG("[Window Thread %ws]: Flags: %c%c%c%c%c%c%c%c\n" COMMA client->address COMMA flags & 0x80 ? '1' : '0' COMMA flags & 0x40 ? '1' : '0' COMMA flags & 0x20 ? '1' : '0' COMMA flags & 0x10 ? '1' : '0' COMMA flags & 0x08 ? '1' : '0' COMMA flags & 0x04 ? '1' : '0' COMMA flags & 0x02 ? '1' : '0' COMMA flags & 0x01 ? '1' : '0');
	PACKET packet;
	packet.size = 1;
	packet.packetType = PACKET_TYPE_STREAM_FRAME;
	packet.data = &flags;

	if(!SendPacket(client->socket, &packet)) {
		DWORD error = GetLastError();

		if(error != WSAECONNRESET) {
			KHWin32DialogErrorW(error, L"SendPacket");
		}
	}
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
	case WM_SIZE: {
		unsigned int width = LOWORD(lparam);
		unsigned int height = HIWORD(lparam);

		if(!width || !height) {
			return 0;
		}

		WaitForSingleObject(client->stream->lock, INFINITE);
		unsigned int newWidth = (unsigned int) (((double) client->stream->width) / ((double) client->stream->height) * ((double) height));
		unsigned int newHeight = (unsigned int) (((double) client->stream->height) / ((double) client->stream->width) * ((double) width));

		if(newWidth < width) {
			newHeight = height;
		} else {
			newWidth = width;
		}

		client->stream->x = newWidth < width ? (int) ((((double) width) - ((double) newWidth)) / 2.0) : 0;
		client->stream->y = newWidth < width ? 0 : (int) ((((double) height) - ((double) newHeight)) / 2.0);
		client->stream->imageWidth = newWidth;
		client->stream->imageHeight = newHeight;
		ReleaseMutex(client->stream->lock);
		return 0;
	}
	case WM_PAINT: {
		PAINTSTRUCT paintStruct;
		HDC context = BeginPaint(window, &paintStruct);
		HDC bufferContext = CreateCompatibleDC(context);
		RECT bounds;
		GetClientRect(window, &bounds);
		HBITMAP bitmap = CreateCompatibleBitmap(context, bounds.right - bounds.left, bounds.bottom - bounds.top);
		HBITMAP oldBitmap = SelectObject(bufferContext, bitmap);
		HBRUSH brush = GetStockObject(DC_BRUSH);
		SetDCBrushColor(bufferContext, 0x000000);
		FillRect(bufferContext, &bounds, brush);
		WaitForSingleObject(client->stream->lock, INFINITE);

		if(client->stream->pixels) {
			SetStretchBltMode(bufferContext, HALFTONE);
			BITMAPINFO information = {0};
			information.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			information.bmiHeader.biWidth = client->stream->width;
			information.bmiHeader.biHeight = client->stream->height;
			information.bmiHeader.biPlanes = 1;
			information.bmiHeader.biBitCount = 32;
			StretchDIBits(bufferContext, client->stream->x, client->stream->y, client->stream->imageWidth, client->stream->imageHeight, 0, 0, client->stream->width, client->stream->height, client->stream->pixels, &information, DIB_RGB_COLORS, SRCCOPY);
		}

		ReleaseMutex(client->stream->lock);
		BitBlt(context, 0, 0, bounds.right - bounds.left, bounds.bottom - bounds.top, bufferContext, 0, 0, SRCCOPY);
		SelectObject(bufferContext, oldBitmap);
		DeleteObject(bitmap);
		DeleteDC(bufferContext);
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

		WaitForSingleObject(client->stream->lock, INFINITE);
		AppendMenuW(streamingMenu, MF_STRING | (client->stream->streaming ? MF_CHECKED : MF_UNCHECKED), IDM_WINDOW_STREAMING_ENABLE, L"Enable");
		AppendMenuW(sendMethod, MF_STRING | (client->stream->method == SEND_METHOD_FULL ? MF_CHECKED : MF_UNCHECKED), IDM_WINDOW_SEND_METHOD_FULL, L"Full");
		AppendMenuW(sendMethod, MF_STRING | (client->stream->method == SEND_METHOD_BOUNDARY ? MF_CHECKED : MF_UNCHECKED), IDM_WINDOW_SEND_METHOD_BOUNDARY, L"Boundary Differences");
		AppendMenuW(sendMethod, MF_STRING | (client->stream->method == SEND_METHOD_COLOR ? MF_CHECKED : MF_UNCHECKED), IDM_WINDOW_SEND_METHOD_COLOR, L"Color Differences");
		AppendMenuW(sendMethod, MF_STRING | (client->stream->method == SEND_METHOD_UNCOMPRESSED ? MF_CHECKED : MF_UNCHECKED), IDM_WINDOW_SEND_METHOD_UNCOMPRESSED, L"Uncompressed");
		AppendMenuW(streamingMenu, MF_POPUP | (client->stream->streaming ? MF_ENABLED : MF_DISABLED), (UINT_PTR) sendMethod, L"Send Method");
		AppendMenuW(popupMenu, MF_POPUP, (UINT_PTR) streamingMenu, L"Streaming");
		AppendMenuW(popupMenu, MF_STRING | (client->stream->pictureInPicture ? MF_CHECKED : MF_UNCHECKED), IDM_PICTURE_IN_PICTURE, L"Picture in Picture Mode");
		AppendMenuW(popupMenu, MF_STRING, IDM_WINDOW_EXIT, L"Exit");
		SetForegroundWindow(window);
		ReleaseMutex(client->stream->lock);
		BOOL response = TrackPopupMenuEx(popupMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, LOWORD(lparam), HIWORD(lparam), window, NULL);
		DestroyMenu(sendMethod);
		DestroyMenu(streamingMenu);
		DestroyMenu(popupMenu);

		switch(response) {
		case IDM_PICTURE_IN_PICTURE:
			WaitForSingleObject(client->stream->lock, INFINITE);
			client->stream->pictureInPicture = !client->stream->pictureInPicture;
			ReleaseMutex(client->stream->lock);
			break;
		case IDM_WINDOW_EXIT:
			LOG("[Window Thread %ws]: Exiting\n" COMMA client->address);
			ClientDisconnect(client);
			break;
		case IDM_WINDOW_STREAMING_ENABLE:
			WaitForSingleObject(client->stream->lock, INFINITE);
			client->stream->streaming = !client->stream->streaming;
			sendStreamCode(client);
			ReleaseMutex(client->stream->lock);
			break;
		case IDM_WINDOW_SEND_METHOD_FULL:
		case IDM_WINDOW_SEND_METHOD_BOUNDARY:
		case IDM_WINDOW_SEND_METHOD_COLOR:
		case IDM_WINDOW_SEND_METHOD_UNCOMPRESSED:
			WaitForSingleObject(client->stream->lock, INFINITE);
			client->stream->method = response == IDM_WINDOW_SEND_METHOD_FULL ? SEND_METHOD_FULL : response == IDM_WINDOW_SEND_METHOD_BOUNDARY ? SEND_METHOD_BOUNDARY : response == IDM_WINDOW_SEND_METHOD_COLOR ? SEND_METHOD_COLOR : SEND_METHOD_UNCOMPRESSED;
			sendStreamCode(client);
			ReleaseMutex(client->stream->lock);
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
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
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
	client->stream->method = SEND_METHOD_COLOR;
	client->stream->lock = CreateMutexExW(NULL, NULL, 0, DELETE | SYNCHRONIZE);

	if(!client->stream->lock) {
		KHWin32DialogErrorW(GetLastError(), L"CreateMutexExW");
		goto freeStream;
	}

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
