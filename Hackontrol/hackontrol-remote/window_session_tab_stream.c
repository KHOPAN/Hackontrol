#include "window_session_tabs.h"

#define CLASS_NAME L"HackontrolRemoteSessionTabStream"

extern HFONT font;

typedef struct {
	PCLIENT client;
	int buttonWidth;
	int buttonHeight;
	HWND button;
} TABSTREAMDATA, *PTABSTREAMDATA;

static HWND __stdcall clientInitialize(const HWND parent, const PCLIENT client) {
	PTABSTREAMDATA data = KHOPAN_ALLOCATE(sizeof(TABSTREAMDATA));

	if(KHOPAN_ALLOCATE_FAILED(data)) {
		KHOPANERRORCONSOLE_WIN32(KHOPAN_ALLOCATE_ERROR, KHOPAN_ALLOCATE_FUNCTION);
		return NULL;
	}

	data->client = client;
	HWND window = CreateWindowExW(0L, CLASS_NAME, L"", WS_CHILD, 0, 0, 0, 0, parent, NULL, NULL, data);

	if(!window) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		KHOPAN_DEALLOCATE(data);
		return NULL;
	}

	data->buttonWidth = (int) (GetSystemMetrics(SM_CXSCREEN) * 0.0732064422);
	data->buttonHeight = (int) (GetSystemMetrics(SM_CYSCREEN) * 0.0325520833);
	data->button = CreateWindowExW(0L, L"Button", L"Open Stream", WS_CHILD | WS_VISIBLE, 0, 0, data->buttonWidth, data->buttonHeight, window, NULL, NULL, NULL);

	if(!data->button) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		KHOPAN_DEALLOCATE(data);
		return window;
	}

	SendMessageW(data->button, WM_SETFONT, (WPARAM) font, TRUE);
	return window;
}

static LRESULT CALLBACK windowProcedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	USERDATA(PTABSTREAMDATA, data, window, message, wparam, lparam);
	RECT bounds;

	switch(message) {
	case WM_SIZE:
		GetClientRect(window, &bounds);
		SetWindowPos(data->button, NULL, (bounds.right - bounds.left - data->buttonWidth) / 2, (bounds.bottom - bounds.top - data->buttonHeight) / 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		return 0;
	case WM_DESTROY:
		KHOPAN_DEALLOCATE(data);
		return 0;
	}

	return DefWindowProcW(window, message, wparam, lparam);
}

void __stdcall WindowSessionTabStream(const PTABINITIALIZER tab) {
	tab->name = L"Stream";
	tab->clientInitialize = clientInitialize;
	tab->windowClass.lpfnWndProc = windowProcedure;
	tab->windowClass.lpszClassName = CLASS_NAME;
}
