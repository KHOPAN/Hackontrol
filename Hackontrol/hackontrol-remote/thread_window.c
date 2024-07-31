#include "thread_window.h"
#include "window_main.h"
#include "logger.h"

DWORD WINAPI WindowThread(_In_ LPVOID parameter) {
	LOG("[Window Thread]: Hello from window thread\n");
	HWND window = CreateWindowExW(0L, CLASS_CLIENT_WINDOW, L"Client Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 200, 200, NULL, NULL, NULL, NULL);
	MSG message;

	while(GetMessageW(&message, window, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	return 0;
}
