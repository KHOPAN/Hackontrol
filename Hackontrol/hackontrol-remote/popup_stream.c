#include "popup_stream.h"

#define CLASS_NAME L"HackontrolRemotePopupStream"

extern HINSTANCE instance;

typedef struct {
	HANDLE thread;
	HWND window;
} POPUPSTREAMDATA, *PPOPUPSTREAMDATA;

static LRESULT CALLBACK procedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	switch(message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
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
