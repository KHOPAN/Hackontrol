#include "window_session_tabs.h"

HWND __stdcall WindowSessionTabStream(const HWND parent, const PCLIENT client) {
	HWND window = CreateWindowExW(0L, REMOTE_CLASS_TAB_STREAM, L"", WS_CHILD, 0, 0, 0, 0, parent, NULL, NULL, NULL);
	return window;
}

LRESULT CALLBACK WindowSessionTabStreamProcedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	return DefWindowProcW(window, message, wparam, lparam);
}
