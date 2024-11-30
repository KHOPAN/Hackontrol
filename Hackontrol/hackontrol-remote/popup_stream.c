#include "popup_stream.h"

#define CLASS_NAME L"HackontrolRemotePopupStream"

extern HINSTANCE instance;

typedef struct {
	HANDLE thread;
	HWND window;
} POPUPSTREAMDATA, *PPOPUPSTREAMDATA;

static LRESULT CALLBACK procedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	PAINTSTRUCT paintStruct;
	HDC context;
	HDC memoryContext;
	RECT bounds;
	HBITMAP bitmap;
	HBITMAP oldBitmap;
	HBRUSH brush;

	switch(message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_MOUSEMOVE:
		SetCursor(LoadCursorW(NULL, IDC_ARROW));
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

		/*if(data->stream.pixels && WaitForSingleObject(data->mutex, INFINITE) != WAIT_FAILED) {
			information.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			information.bmiHeader.biWidth = data->stream.targetWidth;
			information.bmiHeader.biHeight = data->stream.targetHeight;
			information.bmiHeader.biPlanes = 1;
			information.bmiHeader.biBitCount = 32;
			SetStretchBltMode(memoryContext, HALFTONE);
			StretchDIBits(memoryContext, data->stream.matchAspectRatio && !data->stream.fullscreen ? bounds.left : data->stream.renderX, data->stream.matchAspectRatio && !data->stream.fullscreen ? bounds.top : data->stream.renderY, data->stream.matchAspectRatio && !data->stream.fullscreen ? bounds.right - bounds.left : data->stream.renderWidth, data->stream.matchAspectRatio && !data->stream.fullscreen ? bounds.bottom - bounds.top : data->stream.renderHeight, 0, 0, data->stream.targetWidth, data->stream.targetHeight, data->stream.pixels, &information, DIB_RGB_COLORS, SRCCOPY);
			ReleaseMutex(data->mutex);
		}*/

		BitBlt(context, 0, 0, bounds.right, bounds.bottom, memoryContext, 0, 0, SRCCOPY);
		SelectObject(memoryContext, oldBitmap);
		DeleteObject(bitmap);
		DeleteDC(memoryContext);
		EndPaint(window, &paintStruct);
		return 0;
	}

	return DefWindowProcW(window, message, wparam, lparam);
}

BOOLEAN PopupStreamInitialize() {
	WNDCLASSEXW windowClass = {0};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = procedure;
	windowClass.hInstance = instance;
	windowClass.hCursor = NULL;
	windowClass.hbrBackground = (HBRUSH) (COLOR_MENU + 1);
	windowClass.lpszClassName = CLASS_NAME;

	if(!RegisterClassExW(&windowClass)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"RegisterClassExW");
		return FALSE;
	}

	return TRUE;
}

static DWORD WINAPI thread(_In_ PPOPUPSTREAMDATA data) {
	if(!data) {
		return 1;
	}

	data->window = CreateWindowExW(WS_EX_TOPMOST, CLASS_NAME, NULL, WS_POPUP | WS_VISIBLE, 0, 0, (int) (((double) GetSystemMetrics(SM_CXSCREEN)) * 0.32942899), (int) (((double) GetSystemMetrics(SM_CYSCREEN)) * 0.390625), NULL, NULL, instance, data);
	DWORD codeExit = 1;

	if(!data->window) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		goto functionExit;
	}

	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	codeExit = 0;
functionExit:
	CloseHandle(data->thread);
	return codeExit;
}

BOOLEAN PopupStreamSessionInitialize(const PPOPUPSTREAMSESSION session) {
	if(!session) {
		return FALSE;
	}

	PPOPUPSTREAMDATA data = KHOPAN_ALLOCATE(sizeof(POPUPSTREAMDATA));

	if(!data) {
		KHOPANERRORCONSOLE_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"KHOPAN_ALLOCATE");
		return FALSE;
	}

	data->thread = CreateThread(NULL, 0, thread, data, 0, NULL);

	if(!data->thread) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateThread");
		KHOPAN_DEALLOCATE(data);
		return FALSE;
	}

	if(*session) {
		PopupStreamSessionCleanup(session);
	}

	*session = (POPUPSTREAMSESSION) data;
	return TRUE;
}

void PopupStreamSessionCleanup(const PPOPUPSTREAMSESSION session) {
	if(!session) {
		return;
	}

	PPOPUPSTREAMDATA data = (PPOPUPSTREAMDATA) *session;

	if(!data) {
		return;
	}

	PostMessageW(data->window, WM_CLOSE, 0, 0);
	WaitForSingleObject(data->thread, INFINITE);
	KHOPAN_DEALLOCATE(data);
}

void PopupStreamCleanup() {
	UnregisterClassW(CLASS_NAME, instance);
}
