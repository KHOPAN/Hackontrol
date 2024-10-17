#include "window_session_tabs.h"

HWND __stdcall WindowSessionTabStream(const HWND parent) {
	return CreateWindowExW(0L, L"Button", L"Button", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, parent, NULL, NULL, NULL);
}
