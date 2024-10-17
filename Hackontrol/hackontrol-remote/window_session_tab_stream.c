#include "window_session_tabs.h"

extern HFONT font;

typedef struct {
	HWND button;
	PCLIENT client;
} TABSTREAMDATA, *PTABSTREAMDATA;

HWND __stdcall WindowSessionTabStream(const HWND parent, const PCLIENT client) {
	PTABSTREAMDATA data = KHOPAN_ALLOCATE(sizeof(TABSTREAMDATA));

	if(KHOPAN_ALLOCATE_FAILED(data)) {
		KHOPANERRORCONSOLE_WIN32(KHOPAN_ALLOCATE_ERROR, KHOPAN_ALLOCATE_FUNCTION);
		return NULL;
	}

	data->client = client;
	HWND window = CreateWindowExW(0L, REMOTE_CLASS_TAB_STREAM, L"", WS_CHILD, 0, 0, 0, 0, parent, NULL, NULL, data);

	if(!window) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		KHOPAN_DEALLOCATE(data);
		return NULL;
	}

	data->button = CreateWindowExW(0L, L"Button", L"Open Stream", WS_CHILD, 0, 0, 0, 0, window, NULL, NULL, NULL);

	if(!data->button) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		KHOPAN_DEALLOCATE(data);
		return window;
	}

	SendMessageW(data->button, WM_SETFONT, (WPARAM) font, TRUE);
	return window;
}

LRESULT CALLBACK WindowSessionTabStreamProcedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	USERDATA(PTABSTREAMDATA, data, window, message, wparam, lparam);
	RECT bounds;

	switch(message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_SIZE:
		GetClientRect(window, &bounds);
		LOG("Width: %d Height: %d\n", bounds.right - bounds.left, bounds.bottom - bounds.top);
		return 0;
	}

	return DefWindowProcW(window, message, wparam, lparam);
}
