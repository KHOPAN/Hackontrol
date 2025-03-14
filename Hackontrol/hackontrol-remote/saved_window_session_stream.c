#include "window_session.h"
#include "popup_stream.h"
#include "hrsp_remote.h"

#define CLASS_NAME        L"HackontrolRemoteSessionTabStream"
//#define CLASS_NAME_STREAM L"HackontrolRemoteSessionStream"

extern HINSTANCE instance;
extern HFONT font;

typedef struct {
	PCLIENT client;
	HANDLE mutex;
	int buttonWidth;
	int buttonHeight;
	int activationDistance;
	int minimumSize;
	HWND button;
	POPUPSTREAMSESSION popup;

	/*struct {
		HANDLE thread;
		int minimumWidth;
		int minimumHeight;
		HWND window;
		LONGLONG lastTime;
		LONGLONG lastUpdate;
		ULONGLONG totalTime;
		ULONGLONG totalTimes;
		BOOL stream;
		SENDMETHOD method;
		BOOL fullscreen;
		BOOL limitToScreen;
		BOOL lockFrame;
		BOOL matchAspectRatio;
		LONG_PTR windowStyle;
		WINDOWPLACEMENT windowPlacement;
		UINT targetWidth;
		UINT targetHeight;
		PBYTE pixels;
		UINT renderWidth;
		UINT renderHeight;
		UINT renderX;
		UINT renderY;
		BOOL cursorNorth;
		BOOL cursorEast;
		BOOL cursorSouth;
		BOOL cursorWest;
		RECT pressedBounds;
		POINT pressedLocation;
	} stream;*/
} TABSTREAMDATA, *PTABSTREAMDATA;

/*static void __stdcall uninitialize(const PULONGLONG data) {
	if(*data) {
		UnregisterClassW(CLASS_NAME_STREAM, instance);
	}
}*/

static HWND __stdcall clientInitialize(const PCLIENT client, const PULONGLONG customData, const HWND parent) {
	PTABSTREAMDATA data = KHOPAN_ALLOCATE(sizeof(TABSTREAMDATA));

	if(!data) {
		KHOPANERRORCONSOLE_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"KHOPAN_ALLOCATE");
		return NULL;
	}

	data->client = client;
	data->mutex = CreateMutexExW(NULL, NULL, 0, SYNCHRONIZE | DELETE);

	if(!data->mutex) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateMutexExW");
		KHOPAN_DEALLOCATE(data);
		return NULL;
	}

	HWND window = CreateWindowExW(WS_EX_CONTROLPARENT, CLASS_NAME, L"", WS_CHILD, 0, 0, 0, 0, parent, NULL, NULL, data);

	if(!window) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		CloseHandle(data->mutex);
		KHOPAN_DEALLOCATE(data);
		return NULL;
	}

	double width = GetSystemMetrics(SM_CXSCREEN);
	data->buttonWidth = (int) (width * 0.0732064422);
	data->buttonHeight = (int) (((double) GetSystemMetrics(SM_CYSCREEN)) * 0.0325520833);
	data->activationDistance = (int) (width * 0.00878477306);
	data->minimumSize = (int) (width * 0.0263543192);
	data->button = CreateWindowExW(0L, L"Button", L"Open Stream", WS_TABSTOP | WS_CHILD | WS_VISIBLE, 0, 0, data->buttonWidth, data->buttonHeight, window, NULL, NULL, NULL);

	if(!data->button) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		DestroyWindow(window);
		CloseHandle(data->mutex);
		KHOPAN_DEALLOCATE(data);
		return NULL;
	}

	SendMessageW(data->button, WM_SETFONT, (WPARAM) font, TRUE);
	*customData = (ULONGLONG) data;
	return window;
}

/*static DWORD WINAPI threadStream(_In_ PTABSTREAMDATA data) {
	if(!data) {
		return 1;
	}

	data->stream.minimumWidth = data->minimumSize;
	data->stream.minimumHeight = data->minimumSize;
	data->stream.limitToScreen = TRUE;
	data->stream.matchAspectRatio = TRUE;
	data->stream.window = CreateWindowExW(WS_EX_TOPMOST, CLASS_NAME_STREAM, NULL, WS_POPUP | WS_VISIBLE, 0, 0, (int) (((double) GetSystemMetrics(SM_CXSCREEN)) * 0.32942899), (int) (((double) GetSystemMetrics(SM_CYSCREEN)) * 0.390625), NULL, NULL, instance, data);
	DWORD codeExit = 1;

	if(!data->stream.window) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		goto functionExit;
	}

	HRSPPACKET packet = {0};
	packet.type = HRSP_REMOTE_SERVER_STREAM_REQUEST;
	HRSPPacketSend(&data->client->hrsp, &packet, NULL);
	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	//data->stream.stream = FALSE;
	//sendFrameCode(data);
	codeExit = 0;
functionExit:
	WaitForSingleObject(data->mutex, INFINITE);
	ReleaseMutex(data->mutex);
	CloseHandle(data->stream.thread);

	for(size_t i = 0; i < sizeof(data->stream); i++) {
		((PBYTE) &data->stream)[i] = 0;
	}

	return codeExit;
}*/

static LRESULT CALLBACK tabProcedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	USERDATA(PTABSTREAMDATA, data, window, message, wparam, lparam);
	RECT bounds;

	switch(message) {
	case WM_COMMAND:
		if(HIWORD(wparam) != BN_CLICKED) {
			break;
		}

		/*if(data->stream.thread) {
			PostMessageW(data->stream.window, WM_CLOSE, 0, 0);
			WaitForSingleObject(data->stream.thread, INFINITE);
		}

		WaitForSingleObject(data->mutex, INFINITE);
		ReleaseMutex(data->mutex);
		data->stream.thread = CreateThread(NULL, 0, threadStream, data, 0, NULL);

		if(!data->stream.thread) {
			KHOPANLASTERRORCONSOLE_WIN32(L"CreateThread");
			break;
		}*/

		PopupStreamSessionInitialize(&data->popup);
		return 0;
	case WM_CTLCOLORBTN:
		SetDCBrushColor((HDC) wparam, 0xF9F9F9);
		return (LRESULT) GetStockObject(DC_BRUSH);
	case WM_DESTROY:
		/*if(data->stream.thread) {
			PostMessageW(data->stream.window, WM_CLOSE, 0, 0);
			WaitForSingleObject(data->stream.thread, INFINITE);
		}

		WaitForSingleObject(data->mutex, INFINITE);
		CloseHandle(data->mutex);*/
		PopupStreamSessionCleanup(&data->popup);
		KHOPAN_DEALLOCATE(data);
		return 0;
	case WM_SIZE:
		GetClientRect(window, &bounds);
		SetWindowPos(data->button, NULL, (int) ((((double) bounds.right) - ((double) bounds.left) - ((double) data->buttonWidth)) / 2.0), (int) ((((double) bounds.bottom) - ((double) bounds.top) - ((double) data->buttonHeight)) / 2.0), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		return 0;
	}

	return DefWindowProcW(window, message, wparam, lparam);
}

/*static LRESULT CALLBACK streamProcedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	USERDATA(PTABSTREAMDATA, data, window, message, wparam, lparam);
	RECT bounds;
	PAINTSTRUCT paintStruct = {0};
	HDC context;
	HDC memoryContext;
	HBITMAP bitmap;
	HBITMAP oldBitmap;
	HBRUSH brush;
	BITMAPINFO information = {0};
	HMENU menu;
	HMENU sendMethodMenu = NULL;
	POINT location;
	int screenWidth;
	int screenHeight;
	int temporary;

	switch(message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_COMMAND:
		switch(LOWORD(wparam)) {
		case IDM_STREAM_ENABLE:
			data->stream.stream = !data->stream.stream;
			if(!data->stream.stream) updateTitle(data);
			sendFrameCode(data);
			return 0;
		case IDM_SEND_METHOD_FULL:
		case IDM_SEND_METHOD_BOUNDARY:
		case IDM_SEND_METHOD_COLOR:
		case IDM_SEND_METHOD_UNCOMPRESSED:
			data->stream.method = LOWORD(wparam) - IDM_SEND_METHOD_FULL + SEND_METHOD_FULL;
			sendFrameCode(data);
			return 0;
		case IDM_ALWAYS_ON_TOP:
			SetWindowPos(window, (GetWindowLongW(window, GWL_EXSTYLE) & WS_EX_TOPMOST) ? HWND_NOTOPMOST : HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
			return 0;
		case IDM_FULLSCREEN:
			data->stream.fullscreen = !data->stream.fullscreen;
			if(data->stream.fullscreen) data->stream.windowStyle = GetWindowLongPtrW(window, GWL_STYLE);
			data->stream.windowPlacement.length = sizeof(WINDOWPLACEMENT);
			(data->stream.fullscreen ? GetWindowPlacement : SetWindowPlacement)(window, &data->stream.windowPlacement);
			SetWindowLongPtrW(window, GWL_STYLE, data->stream.fullscreen ? WS_POPUP | WS_VISIBLE : data->stream.windowStyle);
			if(data->stream.fullscreen) SetWindowPos(window, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED);
			PostMessageW(window, WM_SIZE, 0, 0);
			return 0;
		case IDM_LIMIT_TO_SCREEN:
			data->stream.limitToScreen = !data->stream.limitToScreen;
			if(!data->stream.limitToScreen) return 0;
			GetWindowRect(window, &bounds);
			limitToScreen(data, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), bounds.left, bounds.top, bounds.right, bounds.bottom);
			return 0;
		case IDM_LOCK_FRAME:
			data->stream.lockFrame = !data->stream.lockFrame;
			return 0;
		case IDM_MATCH_ASPECT_RATIO:
			data->stream.matchAspectRatio = !data->stream.matchAspectRatio;
			if(!data->stream.matchAspectRatio) InvalidateRect(window, NULL, FALSE);
			if(!data->stream.matchAspectRatio) return 0;
			matchAspectRatio(data);
			return 0;
		case IDM_MINIMIZE:
			ShowWindow(window, SW_MINIMIZE);
			return 0;
		case IDM_CLOSE:
			PostMessageW(window, WM_CLOSE, 0, 0);
			return 0;
		}

		break;
	case WM_CONTEXTMENU:
		menu = CreatePopupMenu();

		if(!menu) {
			break;
		}

		AppendMenuW(menu, MF_STRING | (data->stream.stream ? MF_CHECKED : MF_UNCHECKED), IDM_STREAM_ENABLE, L"Enable Stream");

		if(sendMethodMenu = CreateMenu()) {
			AppendMenuW(sendMethodMenu, MF_STRING | (data->stream.method == SEND_METHOD_FULL ? MF_CHECKED : MF_UNCHECKED), IDM_SEND_METHOD_FULL, L"Full");
			AppendMenuW(sendMethodMenu, MF_STRING | (data->stream.method == SEND_METHOD_BOUNDARY ? MF_CHECKED : MF_UNCHECKED), IDM_SEND_METHOD_BOUNDARY, L"Boundary Differences");
			AppendMenuW(sendMethodMenu, MF_STRING | (data->stream.method == SEND_METHOD_COLOR ? MF_CHECKED : MF_UNCHECKED), IDM_SEND_METHOD_COLOR, L"Color Differences");
			AppendMenuW(sendMethodMenu, MF_STRING | (data->stream.method == SEND_METHOD_UNCOMPRESSED ? MF_CHECKED : MF_UNCHECKED), IDM_SEND_METHOD_UNCOMPRESSED, L"Uncompressed");
		}

		AppendMenuW(menu, MF_POPUP | (data->stream.stream ? MF_ENABLED : MF_DISABLED), (UINT_PTR) sendMethodMenu, L"Send Method");
		AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
		AppendMenuW(menu, MF_STRING | ((GetWindowLongW(window, GWL_EXSTYLE) & WS_EX_TOPMOST) ? MF_CHECKED : MF_UNCHECKED), IDM_ALWAYS_ON_TOP, L"Always On Top");
		AppendMenuW(menu, MF_STRING | (data->stream.fullscreen ? MF_CHECKED : MF_UNCHECKED), IDM_FULLSCREEN, L"Fullscreen");
		AppendMenuW(menu, MF_STRING | (data->stream.limitToScreen ? MF_CHECKED : MF_UNCHECKED) | (data->stream.fullscreen ? MF_DISABLED : MF_ENABLED), IDM_LIMIT_TO_SCREEN, L"Limit To Screen");
		AppendMenuW(menu, MF_STRING | (data->stream.lockFrame ? MF_CHECKED : MF_UNCHECKED) | (data->stream.fullscreen ? MF_DISABLED : MF_ENABLED), IDM_LOCK_FRAME, L"Lock Frame");
		AppendMenuW(menu, MF_STRING | (data->stream.matchAspectRatio ? MF_CHECKED : MF_UNCHECKED) | (data->stream.fullscreen ? MF_DISABLED : MF_ENABLED), IDM_MATCH_ASPECT_RATIO, L"Match Aspect Ratio");
		AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
		AppendMenuW(menu, MF_STRING, IDM_MINIMIZE, L"Minimize");
		AppendMenuW(menu, MF_STRING, IDM_CLOSE, L"Close");
		SetForegroundWindow(window);
		TrackPopupMenuEx(menu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, LOWORD(lparam), HIWORD(lparam), window, NULL);
		DestroyMenu(menu);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_ERASEBKGND:
		return 1;
	case WM_LBUTTONDOWN:
		GetWindowRect(window, &data->stream.pressedBounds);
		GetCursorPos(&data->stream.pressedLocation);
		SetCapture(window);
		return 0;
	case WM_LBUTTONUP:
		ReleaseCapture();
		return 0;
	case WM_MOUSEMOVE:
		if(data->stream.lockFrame || data->stream.fullscreen) {
			SetCursor(LoadCursorW(NULL, IDC_ARROW));
			break;
		}

		if(!(wparam & MK_LBUTTON)) {
			location.x = GET_X_LPARAM(lparam);
			location.y = GET_Y_LPARAM(lparam);
			GetClientRect(window, &bounds);
			data->stream.cursorNorth = location.y >= 0 && location.y <= data->activationDistance;
			data->stream.cursorEast = location.x >= bounds.right - data->activationDistance && location.x < bounds.right;
			data->stream.cursorSouth = location.y >= bounds.bottom - data->activationDistance && location.y < bounds.bottom;
			data->stream.cursorWest = location.x >= 0 && location.x <= data->activationDistance;
			SetCursor(LoadCursorW(NULL, data->stream.cursorNorth ? data->stream.cursorWest ? IDC_SIZENWSE : data->stream.cursorEast ? IDC_SIZENESW : IDC_SIZENS : data->stream.cursorSouth ? data->stream.cursorWest ? IDC_SIZENESW : data->stream.cursorEast ? IDC_SIZENWSE : IDC_SIZENS : data->stream.cursorWest ? IDC_SIZEWE : data->stream.cursorEast ? IDC_SIZEWE : IDC_ARROW));
			break;
		}

		GetCursorPos(&location);
		screenWidth = GetSystemMetrics(SM_CXSCREEN);
		screenHeight = GetSystemMetrics(SM_CYSCREEN);

		if(!data->stream.cursorNorth && !data->stream.cursorEast && !data->stream.cursorSouth && !data->stream.cursorWest) {
			bounds.left = location.x - data->stream.pressedLocation.x + data->stream.pressedBounds.left;
			bounds.top = location.y - data->stream.pressedLocation.y + data->stream.pressedBounds.top;
			limitToScreen(data, screenWidth, screenHeight, bounds.left, bounds.top, bounds.left + data->stream.pressedBounds.right - data->stream.pressedBounds.left, bounds.top + data->stream.pressedBounds.bottom - data->stream.pressedBounds.top);
			break;
		}

		GetWindowRect(window, &bounds);

		if(data->stream.cursorNorth) {
			bounds.top = location.y - data->stream.pressedLocation.y + data->stream.pressedBounds.top;
			bounds.top = max(bounds.top, 0);
			temporary = data->stream.pressedBounds.bottom - data->stream.minimumHeight;
			bounds.top = min(bounds.top, temporary);

			if(data->stream.matchAspectRatio && data->stream.targetWidth && data->stream.targetHeight) {
				bounds.right = data->stream.pressedBounds.right;
				bounds.left = (int) (((double) data->stream.pressedBounds.right) - ((double) data->stream.targetWidth) / ((double) data->stream.targetHeight) * (((double) data->stream.pressedBounds.bottom) - ((double) bounds.top)));
			}
		}

		if(data->stream.cursorEast && !(data->stream.matchAspectRatio && (data->stream.cursorNorth || data->stream.cursorSouth))) {
			bounds.right = location.x - data->stream.pressedLocation.x + data->stream.pressedBounds.right;
			bounds.right = min(bounds.right, screenWidth);
			temporary = data->stream.pressedBounds.left + data->stream.minimumWidth;
			bounds.right = max(bounds.right, temporary);

			if(data->stream.matchAspectRatio && data->stream.targetWidth && data->stream.targetHeight) {
				bounds.top = data->stream.pressedBounds.top;
				bounds.bottom = (int) (((double) data->stream.targetHeight) / ((double) data->stream.targetWidth) * (((double) bounds.right) - ((double) data->stream.pressedBounds.left)) + ((double) data->stream.pressedBounds.top));
			}
		}

		if(data->stream.cursorSouth) {
			bounds.bottom = location.y - data->stream.pressedLocation.y + data->stream.pressedBounds.bottom;
			bounds.bottom = min(bounds.bottom, screenHeight);
			temporary = data->stream.pressedBounds.top + data->stream.minimumHeight;
			bounds.bottom = max(bounds.bottom, temporary);

			if(data->stream.matchAspectRatio && data->stream.targetWidth && data->stream.targetHeight) {
				bounds.left = data->stream.pressedBounds.left;
				bounds.right = (int) (((double) data->stream.targetWidth) / ((double) data->stream.targetHeight) * (((double) bounds.bottom) - ((double) data->stream.pressedBounds.top)) + ((double) data->stream.pressedBounds.left));
			}
		}

		if(data->stream.cursorWest && !(data->stream.matchAspectRatio && (data->stream.cursorNorth || data->stream.cursorSouth))) {
			bounds.left = location.x - data->stream.pressedLocation.x + data->stream.pressedBounds.left;
			bounds.left = max(bounds.left, 0);
			temporary = data->stream.pressedBounds.right - data->stream.minimumWidth;
			bounds.left = min(bounds.left, temporary);

			if(data->stream.matchAspectRatio && data->stream.targetWidth && data->stream.targetHeight) {
				bounds.bottom = data->stream.pressedBounds.bottom;
				bounds.top = (int) (((double) data->stream.pressedBounds.bottom) - ((double) data->stream.targetHeight) / ((double) data->stream.targetWidth) * (((double) data->stream.pressedBounds.right) - ((double) bounds.left)));
			}
		}

		limitToScreen(data, screenWidth, screenHeight, bounds.left, bounds.top, bounds.right, bounds.bottom);
		return 0;
	case WM_PAINT:
		context = BeginPaint(window, &paintStruct);
		memoryContext = CreateCompatibleDC(context);
		GetClientRect(window, &bounds);
		bitmap = CreateCompatibleBitmap(context, bounds.right, bounds.bottom);
		oldBitmap = SelectObject(memoryContext, bitmap);
		brush = GetStockObject(DC_BRUSH);
		SetDCBrushColor(memoryContext, 0x000000);
		FillRect(memoryContext, &bounds, brush);

		if(data->stream.pixels && WaitForSingleObject(data->mutex, INFINITE) != WAIT_FAILED) {
			information.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			information.bmiHeader.biWidth = data->stream.targetWidth;
			information.bmiHeader.biHeight = data->stream.targetHeight;
			information.bmiHeader.biPlanes = 1;
			information.bmiHeader.biBitCount = 32;
			SetStretchBltMode(memoryContext, HALFTONE);
			StretchDIBits(memoryContext, data->stream.matchAspectRatio && !data->stream.fullscreen ? bounds.left : data->stream.renderX, data->stream.matchAspectRatio && !data->stream.fullscreen ? bounds.top : data->stream.renderY, data->stream.matchAspectRatio && !data->stream.fullscreen ? bounds.right - bounds.left : data->stream.renderWidth, data->stream.matchAspectRatio && !data->stream.fullscreen ? bounds.bottom - bounds.top : data->stream.renderHeight, 0, 0, data->stream.targetWidth, data->stream.targetHeight, data->stream.pixels, &information, DIB_RGB_COLORS, SRCCOPY);
			ReleaseMutex(data->mutex);
		}

		BitBlt(context, 0, 0, bounds.right, bounds.bottom, memoryContext, 0, 0, SRCCOPY);
		SelectObject(memoryContext, oldBitmap);
		DeleteObject(bitmap);
		DeleteDC(memoryContext);
		EndPaint(window, &paintStruct);
		return 0;
	case WM_SIZE:
		GetClientRect(window, &bounds);
		bounds.right -= bounds.left;
		bounds.bottom -= bounds.top;

		if(bounds.right < 1 || bounds.bottom < 1 || !data->stream.targetWidth || !data->stream.targetHeight || WaitForSingleObject(data->mutex, INFINITE) == WAIT_FAILED) {
			break;
		}

		data->stream.renderWidth = (UINT) (((double) data->stream.targetWidth) / ((double) data->stream.targetHeight) * ((double) bounds.bottom));
		data->stream.renderHeight = (UINT) (((double) data->stream.targetHeight) / ((double) data->stream.targetWidth) * ((double) bounds.right));
		temporary = data->stream.renderWidth < ((UINT) bounds.right);

		if(temporary) {
			data->stream.renderHeight = bounds.bottom;
		} else {
			data->stream.renderWidth = bounds.right;
		}

		data->stream.renderX = temporary ? (UINT) ((((double) bounds.right) - ((double) data->stream.renderWidth)) / 2.0) : 0;
		data->stream.renderY = temporary ? 0 : (UINT) ((((double) bounds.bottom) - ((double) data->stream.renderHeight)) / 2.0);
		ReleaseMutex(data->mutex);
		return 0;
	}

	return DefWindowProcW(window, message, wparam, lparam);
}*/

void __stdcall WindowSessionTabStream(const PTABINITIALIZER tab) {
	tab->name = L"Stream";
	//tab->uninitialize = uninitialize;
	tab->clientInitialize = clientInitialize;
	//tab->packetHandler = packetHandler;
	//tab->alwaysProcessPacket = TRUE;
	tab->windowClass.lpfnWndProc = tabProcedure;
	tab->windowClass.lpszClassName = CLASS_NAME;
	/*QueryPerformanceFrequency((PLARGE_INTEGER) &performanceFrequency);
	WNDCLASSEXW windowClass = {0};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = streamProcedure;
	windowClass.hInstance = instance;
	windowClass.hCursor = NULL;
	windowClass.hbrBackground = (HBRUSH) (COLOR_MENU + 1);
	windowClass.lpszClassName = CLASS_NAME_STREAM;
	tab->data = RegisterClassExW(&windowClass);*/
}

/*#include <windowsx.h>
#include <hrsp_remote.h>

#define IDM_STREAM_ENABLE            0xE001
#define IDM_SEND_METHOD_FULL         0xE002
#define IDM_SEND_METHOD_BOUNDARY     0xE003
#define IDM_SEND_METHOD_COLOR        0xE004
#define IDM_SEND_METHOD_UNCOMPRESSED 0xE005
#define IDM_ALWAYS_ON_TOP            0xE006
#define IDM_FULLSCREEN               0xE007
#define IDM_LIMIT_TO_SCREEN          0xE008
#define IDM_LOCK_FRAME               0xE009
#define IDM_MATCH_ASPECT_RATIO       0xE00A
#define IDM_MINIMIZE                 0xE00B
#define IDM_CLOSE                    0xE00C

#define QOI_OP_RGB   0b11111110
#define QOI_OP_INDEX 0b00000000
#define QOI_OP_DIFF  0b01000000
#define QOI_OP_LUMA  0b10000000
#define QOI_OP_RUN   0b11000000
#define OP_MASK      0b11000000

typedef enum {
	SEND_METHOD_FULL = 0,
	SEND_METHOD_BOUNDARY,
	SEND_METHOD_COLOR,
	SEND_METHOD_UNCOMPRESSED,
} SENDMETHOD;

static void updateTitle(const PTABSTREAMDATA data) {
	LPWSTR title = !data->stream.stream || !data->stream.lastTime ? KHOPANFormatMessage(L"Stream [%ws]", data->client->name) : KHOPANFormatMessage(L"Stream [%ws] [%.2lf FPS]", data->client->name, ((long double) data->stream.totalTimes) * ((long double) performanceFrequency) / ((long double) data->stream.totalTime));
	SetWindowTextW(data->stream.window, title ? title : L"Stream");

	if(title) {
		LocalFree(title);
	}
}

static void matchAspectRatio(const PTABSTREAMDATA data) {
	if(!data->stream.targetWidth || !data->stream.targetHeight) {
		return;
	}

	RECT bounds;
	GetClientRect(data->stream.window, &bounds);
	bounds.right -= bounds.left;
	bounds.bottom -= bounds.top;

	if(bounds.right < 1 || bounds.bottom < 1) {
		return;
	}

	bounds.left = (int) (((double) data->stream.targetWidth) / ((double) data->stream.targetHeight) * ((double) bounds.bottom));
	bounds.top = (int) (((double) data->stream.targetHeight) / ((double) data->stream.targetWidth) * ((double) bounds.right));
	SetWindowPos(data->stream.window, HWND_TOP, 0, 0, bounds.left < bounds.right ? bounds.left : bounds.right, bounds.left < bounds.right ? bounds.bottom : bounds.top, SWP_NOMOVE | SWP_FRAMECHANGED);
	PostMessageW(data->stream.window, WM_SIZE, 0, 0);
}

static BOOL __stdcall packetHandler(const PCLIENT client, const PULONGLONG customData, const PHRSPPACKET packet) {
	if(packet->type != HRSP_REMOTE_CLIENT_STREAM_FRAME_PACKET) {
		return FALSE;
	}

	if(!customData || !client->session.tabs) {
		return TRUE;
	}

	PTABSTREAMDATA data = (PTABSTREAMDATA) *customData;

	if(!data->stream.stream || WaitForSingleObject(data->mutex, INFINITE) == WAIT_FAILED) {
		return TRUE;
	}

	UINT width = (packet->data[1] << 24) | (packet->data[2] << 16) | (packet->data[3] << 8) | packet->data[4];
	UINT height = (packet->data[5] << 24) | (packet->data[6] << 16) | (packet->data[7] << 8) | packet->data[8];

	if(!width || !height) {
		goto releaseMutex;
	}

	if(data->stream.targetWidth != width || data->stream.targetHeight != height) {
		data->stream.targetWidth = width;
		data->stream.targetHeight = height;

		if(data->stream.pixels) {
			KHOPAN_DEALLOCATE(data->stream.pixels);
		}

		data->stream.pixels = KHOPAN_ALLOCATE(width * height * 4);

		if(KHOPAN_ALLOCATE_FAILED(data->stream.pixels)) {
			goto releaseMutex;
		}

		double minimum = min(data->stream.targetWidth, data->stream.targetHeight);
		data->stream.minimumWidth = (UINT) (((double) data->stream.targetWidth) / minimum * ((double) data->minimumSize));
		data->stream.minimumHeight = (UINT) (((double) data->stream.targetHeight) / minimum * ((double) data->minimumSize));

		if(data->stream.matchAspectRatio) {
			ReleaseMutex(data->mutex);
			matchAspectRatio(data);
			if(WaitForSingleObject(data->mutex, INFINITE) == WAIT_FAILED) return TRUE;
		} else {
			PostMessageW(data->stream.window, WM_SIZE, 0, 0);
		}
	}

	if(!data->stream.pixels) {
		goto releaseMutex;
	}

	BOOL boundaryDifference = packet->data[0] & 1;
	BOOL colorDifference = (packet->data[0] >> 1) & 1;
	UINT x;
	UINT y;
	UINT pixelIndex;
	UINT dataIndex;

	if(!boundaryDifference || !colorDifference) {
		goto rawPixelExit;
	}

	if(((UINT) packet->size) < width * height * 3 + 9) {
		goto releaseMutex;
	}

	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++) {
			pixelIndex = (y * width + x) * 4;
			dataIndex = ((height - y - 1) * width + x) * 3;
			data->stream.pixels[pixelIndex] = packet->data[dataIndex + 11];
			data->stream.pixels[pixelIndex + 1] = packet->data[dataIndex + 10];
			data->stream.pixels[pixelIndex + 2] = packet->data[dataIndex + 9];
		}
	}

	goto updateFrame;
rawPixelExit:
	if(boundaryDifference && packet->size - 9 < 16) {
		goto releaseMutex;
	}

	UINT startX = boundaryDifference ? (packet->data[9] << 24) | (packet->data[10] << 16) | (packet->data[11] << 8) | packet->data[12] : 0;
	UINT endX = boundaryDifference ? (packet->data[17] << 24) | (packet->data[18] << 16) | (packet->data[19] << 8) | packet->data[20] : width - 1;
	UINT endY = boundaryDifference ? (packet->data[21] << 24) | (packet->data[22] << 16) | (packet->data[23] << 8) | packet->data[24] : height - 1;
	BYTE seenRed[64];
	BYTE seenGreen[64];
	BYTE seenBlue[64];
	size_t pointer;

	for(pointer = 0; pointer < 64; pointer++) {
		seenRed[pointer] = 0;
		seenGreen[pointer] = 0;
		seenBlue[pointer] = 0;
	}

	pointer = boundaryDifference ? 25 : 9;
	UINT red = 0;
	UINT green = 0;
	UINT blue = 0;
	UINT run = 0;
	int temporary = 0;
#define APPLY_COLOR if(colorDifference){data->stream.pixels[pixelIndex]-=blue;data->stream.pixels[pixelIndex+1]-=green;data->stream.pixels[pixelIndex+2]-=red;}else{data->stream.pixels[pixelIndex]=blue;data->stream.pixels[pixelIndex+1]=green;data->stream.pixels[pixelIndex+2]=red;}

	for(y = boundaryDifference ? (packet->data[13] << 24) | (packet->data[14] << 16) | (packet->data[15] << 8) | packet->data[16] : 0; y <= endY; y++) {
		for(x = startX; x <= endX; x++) {
			pixelIndex = ((height - y - 1) * width + x) * 4;

			if(run > 0) {
				APPLY_COLOR;
				run--;
				continue;
			}

			if(packet->size - pointer < 1) goto releaseMutex;
			temporary = packet->data[pointer++];

			if(temporary == QOI_OP_RGB) {
				if(packet->size - pointer < 3) goto releaseMutex;
				red = packet->data[pointer++];
				green = packet->data[pointer++];
				blue = packet->data[pointer++];
				dataIndex = (red * 3 + green * 5 + blue * 7 + 0xFF * 11) & 0b111111;
				seenRed[dataIndex] = red;
				seenGreen[dataIndex] = green;
				seenBlue[dataIndex] = blue;
				APPLY_COLOR;
				continue;
			}

			switch(temporary & OP_MASK) {
			case QOI_OP_INDEX:
				dataIndex = temporary & 0b111111;
				red = seenRed[dataIndex];
				green = seenGreen[dataIndex];
				blue = seenBlue[dataIndex];
				break;
			case QOI_OP_DIFF:
				red += ((temporary >> 4) & 0b11) - 2;
				green += ((temporary >> 2) & 0b11) - 2;
				blue += (temporary & 0b11) - 2;
				break;
			case QOI_OP_LUMA:
				if(packet->size - pointer < 1) goto releaseMutex;
				dataIndex = packet->data[pointer++];
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
updateFrame:
	ReleaseMutex(data->mutex);
	InvalidateRect(data->stream.window, NULL, FALSE);
	LONGLONG time = 0;
	QueryPerformanceCounter((PLARGE_INTEGER) &time);

	if(!data->stream.lastTime) {
		data->stream.lastTime = time;
		data->stream.lastUpdate = time;
		return TRUE;
	}

	data->stream.totalTime += time - data->stream.lastTime;
	data->stream.totalTimes++;
	data->stream.lastTime = time;

	if((((long double) time) - ((long double) data->stream.lastUpdate)) / ((long double) performanceFrequency) >= 0.25) {
		data->stream.lastUpdate = time;
		updateTitle(data);
		data->stream.totalTime = 0;
		data->stream.totalTimes = 0;
	}

	return TRUE;
releaseMutex:
	ReleaseMutex(data->mutex);
	return TRUE;
}

static void sendFrameCode(const PTABSTREAMDATA data) {
	BYTE byte = ((data->stream.method & 0b11) << 1) | (data->stream.stream ? 0b1001 : 0);
	HRSPPACKET packet;
	packet.size = 1;
	packet.type = HRSP_REMOTE_SERVER_STREAM_CODE_PACKET;
	packet.data = &byte;
	HRSPSendPacket(data->client->socket, &data->client->hrsp, &packet, NULL);
}

static void limitToScreen(const PTABSTREAMDATA data, const int screenWidth, const int screenHeight, int left, int top, int right, int bottom) {
	right -= left;
	bottom -= top;
	right = max(right, data->stream.matchAspectRatio ? data->stream.minimumWidth : data->minimumSize);
	bottom = max(bottom, data->stream.matchAspectRatio ? data->stream.minimumHeight : data->minimumSize);

	if(!data->stream.limitToScreen) {
		goto functionExit;
	}

	left = max(left, 0);
	top = max(top, 0);
	right = min(right, screenWidth);
	bottom = min(bottom, screenHeight);

	if(left + right > screenWidth) {
		left = screenWidth - right;
	}

	if(top + bottom > screenHeight) {
		top = screenHeight - bottom;
	}
functionExit:
	SetWindowPos(data->stream.window, HWND_TOP, left, top, right, bottom, 0);
}*/
