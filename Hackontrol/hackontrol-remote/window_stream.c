#include <hrsp_packet.h>
#include <hrsp_remote.h>
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

#define QOI_OP_RGB   0b11111110
#define QOI_OP_INDEX 0b00000000
#define QOI_OP_DIFF  0b01000000
#define QOI_OP_LUMA  0b10000000
#define QOI_OP_RUN   0b11000000
#define OP_MASK      0b11000000

extern HINSTANCE instance;

static void sendFrameCode(const PCLIENT client) {
	BYTE data = ((client->session.stream.menu.method & 0b11) << 1) | (client->session.stream.menu.stream ? 0b1001 : 0);
	HRSPPACKET packet;
	packet.size = 1;
	packet.type = HRSP_REMOTE_SERVER_STREAM_CODE_PACKET;
	packet.data = &data;
	HRSPSendPacket(client->socket, &client->hrsp, &packet, NULL);
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

		if(client->session.stream.pixels && WaitForSingleObject(client->mutex, INFINITE) != WAIT_FAILED) {
			information.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			information.bmiHeader.biWidth = client->session.stream.sourceWidth;
			information.bmiHeader.biHeight = client->session.stream.sourceHeight;
			information.bmiHeader.biPlanes = 1;
			information.bmiHeader.biBitCount = 32;
			SetStretchBltMode(memoryContext, HALFTONE);
			StretchDIBits(memoryContext, client->session.stream.imageX, client->session.stream.imageY, client->session.stream.imageWidth, client->session.stream.imageHeight, 0, 0, client->session.stream.sourceWidth, client->session.stream.sourceHeight, client->session.stream.pixels, &information, DIB_RGB_COLORS, SRCCOPY);
			ReleaseMutex(client->mutex);
		}

		BitBlt(context, 0, 0, bounds.right, bounds.bottom, memoryContext, 0, 0, SRCCOPY);
		SelectObject(memoryContext, oldBitmap);
		DeleteObject(bitmap);
		DeleteDC(memoryContext);
		EndPaint(window, &paintStruct);
		break;
	case WM_ERASEBKGND:
		return 1;
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
		AppendMenuW(menu, MF_STRING | ((GetWindowLongW(window, GWL_EXSTYLE) & WS_EX_TOPMOST) ? MF_CHECKED : MF_UNCHECKED), IDM_ALWAYS_ON_TOP, L"Always On Top");
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
	case WM_COMMAND:
		if(WaitForSingleObject(client->mutex, INFINITE) == WAIT_FAILED) {
			break;
		}

		switch(LOWORD(wparam)) {
		case IDM_STREAM_ENABLE:
			client->session.stream.menu.stream = !client->session.stream.menu.stream;
			sendFrameCode(client);
			break;
		case IDM_SEND_METHOD_FULL:
			client->session.stream.menu.method = SEND_METHOD_FULL;
			sendFrameCode(client);
			break;
		case IDM_SEND_METHOD_BOUNDARY:
			client->session.stream.menu.method = SEND_METHOD_BOUNDARY;
			sendFrameCode(client);
			break;
		case IDM_SEND_METHOD_COLOR:
			client->session.stream.menu.method = SEND_METHOD_COLOR;
			sendFrameCode(client);
			break;
		case IDM_SEND_METHOD_UNCOMPRESSED:
			client->session.stream.menu.method = SEND_METHOD_UNCOMPRESSED;
			sendFrameCode(client);
			break;
		case IDM_MATCH_ASPECT_RATIO:
			if(client->session.stream.sourceWidth < 1 || client->session.stream.sourceHeight < 1) break;
			GetClientRect(window, &bounds);
			bounds.right -= bounds.left;
			bounds.bottom -= bounds.top;
			if(bounds.right < 1 || bounds.bottom < 1) break;
			bounds.left = (int) (((double) client->session.stream.sourceWidth) / ((double) client->session.stream.sourceHeight) * ((double) bounds.bottom));
			bounds.top = (int) (((double) client->session.stream.sourceHeight) / ((double) client->session.stream.sourceWidth) * ((double) bounds.right));
			location.x = bounds.left < bounds.right;
			bounds.right = location.x ? bounds.left : bounds.right;
			bounds.bottom = location.x ? bounds.bottom : bounds.top;
			bounds.left = 0;
			bounds.top = 0;
			AdjustWindowRect(&bounds, (DWORD) GetWindowLongPtrW(window, GWL_STYLE), FALSE);
			SetWindowPos(window, HWND_TOP, 0, 0, bounds.right - bounds.left, bounds.bottom - bounds.top, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
			PostMessageW(window, WM_SIZE, 0, 0);
			break;
		case IDM_ALWAYS_ON_TOP:
			SetWindowPos(window, (GetWindowLongW(window, GWL_EXSTYLE) & WS_EX_TOPMOST) ? HWND_NOTOPMOST : HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
			break;
		case IDM_FULLSCREEN:
			client->session.stream.menu.fullscreen = !client->session.stream.menu.fullscreen;
			if(client->session.stream.menu.fullscreen) client->session.stream.fullscreen.style = GetWindowLongPtrW(window, GWL_STYLE);
			client->session.stream.fullscreen.placement.length = sizeof(WINDOWPLACEMENT);
			(client->session.stream.menu.fullscreen ? GetWindowPlacement : SetWindowPlacement)(window, &client->session.stream.fullscreen.placement);
			SetWindowLongPtrW(window, GWL_STYLE, client->session.stream.menu.fullscreen ? WS_POPUP | WS_VISIBLE : client->session.stream.fullscreen.style);
			if(client->session.stream.menu.fullscreen) SetWindowPos(window, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED);
			PostMessageW(window, WM_SIZE, 0, 0);
			break;
		case IDM_LIMIT_TO_SCREEN:
			client->session.stream.menu.limitToScreen = !client->session.stream.menu.limitToScreen;
			if(!client->session.stream.menu.limitToScreen) break;
			GetWindowRect(window, &bounds);
			bounds.right -= bounds.left;
			bounds.bottom -= bounds.top;
			location.x = GetSystemMetrics(SM_CXSCREEN);
			location.y = GetSystemMetrics(SM_CYSCREEN);
			bounds.left = max(bounds.left, 0);
			bounds.top = max(bounds.top, 0);
			bounds.right = min(bounds.right, location.x);
			bounds.bottom = min(bounds.bottom, location.y);
			if(bounds.left + bounds.right > location.x) bounds.left = location.x - bounds.right;
			if(bounds.top + bounds.bottom > location.y) bounds.top = location.y - bounds.bottom;
			SetWindowPos(window, HWND_TOP, bounds.left, bounds.top, bounds.right, bounds.bottom, 0);
			break;
		case IDM_LOCK_FRAME:
			client->session.stream.menu.lockFrame = !client->session.stream.menu.lockFrame;
			break;
		case IDM_PICTURE_IN_PICTURE:
			client->session.stream.menu.pictureInPicture = !client->session.stream.menu.pictureInPicture;
			SetWindowLongPtrW(window, GWL_STYLE, (client->session.stream.menu.pictureInPicture ? WS_POPUP : WS_OVERLAPPEDWINDOW) | WS_VISIBLE);
			SetWindowPos(window, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
			PostMessageW(window, WM_SIZE, 0, 0);
			break;
		case IDM_CLOSE_WINDOW:
			PostMessageW(window, WM_CLOSE, 0, 0);
			break;
		}

		ReleaseMutex(client->mutex);
		break;
	case WM_MOUSEMOVE:
		if(WaitForSingleObject(client->mutex, INFINITE) == WAIT_FAILED) {
			break;
		}

		if(client->session.stream.menu.lockFrame || client->session.stream.menu.fullscreen) {
			SetCursor(LoadCursorW(NULL, IDC_ARROW));
			ReleaseMutex(client->mutex);
			break;
		}

		if(wparam == MK_LBUTTON) {
			goto dragging;
		}

		if(client->session.stream.menu.pictureInPicture) {
			location.x = LOWORD(lparam);
			location.y = HIWORD(lparam);
			GetClientRect(window, &bounds);
			client->session.stream.cursorNorth = location.y >= 0 && location.y <= client->session.stream.resizeActivationDistance;
			client->session.stream.cursorEast = location.x >= bounds.right - client->session.stream.resizeActivationDistance && location.x < bounds.right;
			client->session.stream.cursorSouth = location.y >= bounds.bottom - client->session.stream.resizeActivationDistance && location.y < bounds.bottom;
			client->session.stream.cursorWest = location.x >= 0 && location.x <= client->session.stream.resizeActivationDistance;
			context = (HDC) (client->session.stream.cursorNorth ? client->session.stream.cursorWest ? IDC_SIZENWSE : client->session.stream.cursorEast ? IDC_SIZENESW : IDC_SIZENS : client->session.stream.cursorSouth ? client->session.stream.cursorWest ? IDC_SIZENESW : client->session.stream.cursorEast ? IDC_SIZENWSE : IDC_SIZENS : client->session.stream.cursorWest ? IDC_SIZEWE : client->session.stream.cursorEast ? IDC_SIZEWE : IDC_ARROW);
		} else {
			context = (HDC) IDC_ARROW;
		}

		SetCursor(LoadCursorW(NULL, (LPCWSTR) context));
		ReleaseMutex(client->mutex);
		break;
	dragging:
		paintStruct.rcPaint.left = GetSystemMetrics(SM_CXSCREEN);
		paintStruct.rcPaint.top = GetSystemMetrics(SM_CYSCREEN);
		GetCursorPos(&location);

		if((!client->session.stream.cursorNorth && !client->session.stream.cursorEast && !client->session.stream.cursorSouth && !client->session.stream.cursorWest) || !client->session.stream.menu.pictureInPicture) {
			if(!client->session.stream.menu.pictureInPicture) SetCursor(LoadCursorW(NULL, IDC_ARROW));
			location.x -= client->session.stream.position.x - client->session.stream.bounds.left;
			location.y -= client->session.stream.position.y - client->session.stream.bounds.top;
			SetWindowPos(window, HWND_TOP, client->session.stream.menu.limitToScreen ? location.x < 0 ? 0 : location.x + client->session.stream.bounds.right - client->session.stream.bounds.left > paintStruct.rcPaint.left ? paintStruct.rcPaint.left - client->session.stream.bounds.right + client->session.stream.bounds.left : location.x : location.x, client->session.stream.menu.limitToScreen ? location.y < 0 ? 0 : location.y + client->session.stream.bounds.bottom - client->session.stream.bounds.top > paintStruct.rcPaint.top ? paintStruct.rcPaint.top - client->session.stream.bounds.bottom + client->session.stream.bounds.top : location.y : location.y, 0, 0, SWP_NOSIZE);
			ReleaseMutex(client->mutex);
			break;
		}

		paintStruct.rcPaint.right = client->session.stream.resizeActivationDistance * 3;
		GetWindowRect(window, &bounds);
		bounds.right -= bounds.left;
		bounds.bottom -= bounds.top;

		if(client->session.stream.cursorNorth) {
			paintStruct.rcPaint.bottom = location.y - client->session.stream.position.y;
			if(client->session.stream.menu.limitToScreen && client->session.stream.bounds.top + paintStruct.rcPaint.bottom < 0) paintStruct.rcPaint.bottom = -client->session.stream.bounds.top;
			if(client->session.stream.bounds.bottom - client->session.stream.bounds.top - paintStruct.rcPaint.bottom < paintStruct.rcPaint.right) paintStruct.rcPaint.bottom = client->session.stream.bounds.bottom - client->session.stream.bounds.top - paintStruct.rcPaint.right;
			bounds.top = client->session.stream.bounds.top + paintStruct.rcPaint.bottom;
			bounds.bottom = client->session.stream.bounds.bottom - bounds.top;
		}

		if(client->session.stream.cursorEast) {
			paintStruct.rcPaint.bottom = location.x - client->session.stream.position.x;
			if(client->session.stream.menu.limitToScreen && client->session.stream.bounds.right + paintStruct.rcPaint.bottom > paintStruct.rcPaint.left) paintStruct.rcPaint.bottom = paintStruct.rcPaint.left - client->session.stream.bounds.right;
			bounds.right = client->session.stream.bounds.right - client->session.stream.bounds.left + paintStruct.rcPaint.bottom;
			bounds.right = max(bounds.right, paintStruct.rcPaint.right);
		}

		if(client->session.stream.cursorSouth) {
			paintStruct.rcPaint.bottom = location.y - client->session.stream.position.y;
			if(client->session.stream.menu.limitToScreen && client->session.stream.bounds.bottom + paintStruct.rcPaint.bottom > paintStruct.rcPaint.top) paintStruct.rcPaint.bottom = paintStruct.rcPaint.top - client->session.stream.bounds.bottom;
			bounds.bottom = client->session.stream.bounds.bottom - client->session.stream.bounds.top + paintStruct.rcPaint.bottom;
			bounds.bottom = max(bounds.bottom, paintStruct.rcPaint.right);
		}

		if(client->session.stream.cursorWest) {
			paintStruct.rcPaint.bottom = location.x - client->session.stream.position.x;
			if(client->session.stream.menu.limitToScreen && client->session.stream.bounds.left + paintStruct.rcPaint.bottom < 0) paintStruct.rcPaint.bottom = -client->session.stream.bounds.left;
			if(client->session.stream.bounds.right - client->session.stream.bounds.left - paintStruct.rcPaint.bottom < paintStruct.rcPaint.right) paintStruct.rcPaint.bottom = client->session.stream.bounds.right - client->session.stream.bounds.left - paintStruct.rcPaint.right;
			bounds.left = client->session.stream.bounds.left + paintStruct.rcPaint.bottom;
			bounds.right = client->session.stream.bounds.right - bounds.left;
		}

		SetWindowPos(window, HWND_TOP, bounds.left, bounds.top, bounds.right, bounds.bottom, (!client->session.stream.cursorNorth && client->session.stream.cursorEast) || (client->session.stream.cursorSouth && !client->session.stream.cursorWest) ? SWP_NOMOVE : 0);
		ReleaseMutex(client->mutex);
		break;
	case WM_LBUTTONDOWN:
		if(WaitForSingleObject(client->mutex, INFINITE) == WAIT_FAILED) {
			break;
		}

		GetWindowRect(window, &client->session.stream.bounds);
		GetCursorPos(&client->session.stream.position);
		ReleaseMutex(client->mutex);
		SetCapture(window);
		break;
	case WM_LBUTTONUP:
		ReleaseCapture();
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
	client->session.stream.resizeActivationDistance = (int) (((double) screenWidth) * 0.00878477306);
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

	if(client->session.stream.pixels && WaitForSingleObject(client->mutex, INFINITE) != WAIT_FAILED) {
		KHOPAN_DEALLOCATE(client->session.stream.pixels);
		client->session.stream.pixels = NULL;
		ReleaseMutex(client->mutex);
	}
functionExit:
	LOG("[Stream %ws]: Exit with code: %d\n", client->address, codeExit);
	CloseHandle(client->session.stream.thread);
	client->session.stream.thread = NULL;
	return codeExit;
}

void WindowStreamFrame(const PCLIENT client, const PBYTE data, const size_t size) {
	if(!data || size < 9 || !client->session.stream.window) {
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

	int x;
	int y;
	int pixelIndex;
	int dataIndex;

	if(!boundaryDifference || !colorDifference) {
		goto exitRawPixel;
	}

	if(size - 9 < width * height * 3) {
		return;
	}

	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++) {
			pixelIndex = (y * width + x) * 4;
			dataIndex = ((height - y - 1) * width + x) * 3;
			client->session.stream.pixels[pixelIndex] = data[dataIndex + 11];
			client->session.stream.pixels[pixelIndex + 1] = data[dataIndex + 10];
			client->session.stream.pixels[pixelIndex + 2] = data[dataIndex + 9];
		}
	}

	goto invalidateWindow;
exitRawPixel:
	if(boundaryDifference && size - 9 < 16) {
		return;
	}

	int startX = boundaryDifference ? (data[9] << 24) | (data[10] << 16) | (data[11] << 8) | data[12] : 0;
	int endX = boundaryDifference ? (data[17] << 24) | (data[18] << 16) | (data[19] << 8) | data[20] : width - 1;
	int endY = boundaryDifference ? (data[21] << 24) | (data[22] << 16) | (data[23] << 8) | data[24] : height - 1;
	BYTE seenRed[64];
	BYTE seenGreen[64];
	BYTE seenBlue[64];
	memset(seenRed, 0, sizeof(seenRed));
	memset(seenGreen, 0, sizeof(seenGreen));
	memset(seenBlue, 0, sizeof(seenBlue));
	size_t pointer = boundaryDifference ? 25 : 9;
	int red = 0;
	int green = 0;
	int blue = 0;
	int run = 0;
	int temporary = 0;
#define APPLY_COLOR if(colorDifference){client->session.stream.pixels[pixelIndex]-=blue;client->session.stream.pixels[pixelIndex+1]-=green;client->session.stream.pixels[pixelIndex+2]-=red;}else{client->session.stream.pixels[pixelIndex]=blue;client->session.stream.pixels[pixelIndex+1]=green;client->session.stream.pixels[pixelIndex+2]=red;}

	for(y = boundaryDifference ? (data[13] << 24) | (data[14] << 16) | (data[15] << 8) | data[16] : 0; y <= endY; y++) {
		for(x = startX; x <= endX; x++) {
			pixelIndex = ((height - y - 1) * width + x) * 4;

			if(run > 0) {
				APPLY_COLOR;
				run--;
				continue;
			}

			if(size - pointer < 1) {
				return;
			}

			temporary = data[pointer++];

			if(temporary == QOI_OP_RGB) {
				if(size - pointer < 3) return;
				red = data[pointer++];
				green = data[pointer++];
				blue = data[pointer++];
				dataIndex = (red * 3 + green * 5 + blue * 7 + 0xFF * 11) & 0b111111;
				seenRed[dataIndex] = red;
				seenGreen[dataIndex] = green;
				seenBlue[dataIndex] = blue;
				APPLY_COLOR;
				continue;
			}

			switch(temporary & OP_MASK) {
			case QOI_OP_INDEX: {
				dataIndex = temporary & 0b111111;
				red = seenRed[dataIndex];
				green = seenGreen[dataIndex];
				blue = seenBlue[dataIndex];
				break;
			}
			case QOI_OP_DIFF:
				red += ((temporary >> 4) & 0b11) - 2;
				green += ((temporary >> 2) & 0b11) - 2;
				blue += (temporary & 0b11) - 2;
				break;
			case QOI_OP_LUMA:
				if(size - pointer < 1) return;
				dataIndex = data[pointer++];
				temporary = (temporary & 0b111111) - 32;
				red += temporary - 8 + ((dataIndex >> 4) & 0b1111);
				green += temporary;
				blue += temporary - 8 + (dataIndex & 0b1111);
				break;
			case QOI_OP_RUN:
				APPLY_COLOR;
				run = (temporary & 0b111111);
				continue;
			}

			dataIndex = (red * 3 + green * 5 + blue * 7 + 0xFF * 11) & 0b111111;
			seenRed[dataIndex] = red;
			seenGreen[dataIndex] = green;
			seenBlue[dataIndex] = blue;
			APPLY_COLOR;
		}
	}
invalidateWindow:
	InvalidateRect(client->session.stream.window, NULL, FALSE);
}

void WindowStreamClose(const PCLIENT client) {
	if(client->session.stream.window) {
		PostMessageW(client->session.stream.window, WM_CLOSE, 0, 0);
	}
}
