#include "window_session_tabs.h"

#define CLASS_NAME        L"HackontrolRemoteSessionTabStream"
#define CLASS_NAME_STREAM L"HackontrolRemoteSessionStream"

extern HINSTANCE instance;
extern HFONT font;

typedef struct {
	PCLIENT client;
	HANDLE mutex;
	int buttonWidth;
	int buttonHeight;
	HWND button;
	HWND window;
	HANDLE thread;
} TABSTREAMDATA, *PTABSTREAMDATA;

static void __stdcall uninitialize() {
	UnregisterClassW(CLASS_NAME_STREAM, instance);
}

static HWND __stdcall clientInitialize(const PCLIENT client, const HWND parent) {
	PTABSTREAMDATA data = KHOPAN_ALLOCATE(sizeof(TABSTREAMDATA));

	if(KHOPAN_ALLOCATE_FAILED(data)) {
		KHOPANERRORCONSOLE_WIN32(KHOPAN_ALLOCATE_ERROR, KHOPAN_ALLOCATE_FUNCTION);
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

	data->buttonWidth = (int) (((double) GetSystemMetrics(SM_CXSCREEN)) * 0.0732064422);
	data->buttonHeight = (int) (((double) GetSystemMetrics(SM_CYSCREEN)) * 0.0325520833);
	data->button = CreateWindowExW(0L, L"Button", L"Open Stream", WS_TABSTOP | WS_CHILD | WS_VISIBLE, 0, 0, data->buttonWidth, data->buttonHeight, window, NULL, NULL, NULL);

	if(!data->button) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		DestroyWindow(window);
		CloseHandle(data->mutex);
		KHOPAN_DEALLOCATE(data);
		return NULL;
	}

	SendMessageW(data->button, WM_SETFONT, (WPARAM) font, TRUE);
	return window;
}

static BOOL __stdcall packetHandler(const PCLIENT client, const PHRSPPACKET packet) {
	LOG("Packet: %u\n", packet->type);
	return FALSE;
}

static DWORD WINAPI threadStream(_In_ PTABSTREAMDATA data) {
	if(!data) {
		return 1;
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int width = (int) (((double) screenWidth) * 0.439238653);
	int height = (int) (((double) screenHeight) * 0.520833333);
	data->window = CreateWindowExW(WS_EX_TOPMOST, CLASS_NAME_STREAM, L"Stream", WS_OVERLAPPEDWINDOW | WS_VISIBLE, (screenWidth - width) / 2, (screenHeight - height) / 2, width, height, NULL, NULL, instance, data);
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
	data->window = NULL;
	data->thread = NULL;
	return codeExit;
}

static LRESULT CALLBACK tabProcedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	USERDATA(PTABSTREAMDATA, data, window, message, wparam, lparam);
	RECT bounds;

	switch(message) {
	case WM_SIZE:
		GetClientRect(window, &bounds);
		SetWindowPos(data->button, NULL, (bounds.right - bounds.left - data->buttonWidth) / 2, (bounds.bottom - bounds.top - data->buttonHeight) / 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		return 0;
	case WM_CTLCOLORBTN:
		SetDCBrushColor((HDC) wparam, 0xF9F9F9);
		return (LRESULT) GetStockObject(DC_BRUSH);
	case WM_COMMAND:
		if(HIWORD(wparam) != BN_CLICKED) {
			break;
		}

		if(data->thread) {
			PostMessageW(data->window, WM_CLOSE, 0, 0);
			WaitForSingleObject(data->thread, INFINITE);
		}

		WaitForSingleObject(data->mutex, INFINITE);
		ReleaseMutex(data->mutex);
		data->thread = CreateThread(NULL, 0, threadStream, data, 0, NULL);

		if(!data->thread) {
			KHOPANLASTERRORCONSOLE_WIN32(L"CreateMutexExW");
			break;
		}

		return 0;
	case WM_DESTROY:
		if(data->thread) {
			PostMessageW(data->window, WM_CLOSE, 0, 0);
			WaitForSingleObject(data->thread, INFINITE);
		}

		WaitForSingleObject(data->mutex, INFINITE);
		CloseHandle(data->mutex);
		KHOPAN_DEALLOCATE(data);
		return 0;
	}

	return DefWindowProcW(window, message, wparam, lparam);
}

static LRESULT CALLBACK streamProcedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	USERDATA(PTABSTREAMDATA, data, window, message, wparam, lparam);

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

void __stdcall WindowSessionTabStream(const PTABINITIALIZER tab) {
	tab->name = L"Stream";
	tab->uninitialize = uninitialize;
	tab->clientInitialize = clientInitialize;
	tab->packetHandler = packetHandler;
	tab->windowClass.lpfnWndProc = tabProcedure;
	tab->windowClass.lpszClassName = CLASS_NAME;
	WNDCLASSEXW windowClass = {0};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.lpfnWndProc = streamProcedure;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH) (COLOR_MENU + 1);
	windowClass.lpszClassName = CLASS_NAME_STREAM;
	RegisterClassExW(&windowClass);
}
