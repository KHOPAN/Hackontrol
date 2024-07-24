#include <khopanwin32.h>

#define HACKONTROL_REMOTE L"HackontrolRemote"

static LRESULT CALLBACK hackontrolRemoteProcedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
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

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE previousInstance, _In_ LPSTR argument, _In_ int commandLineShow) {
	WNDCLASSEXW windowClass = {0};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.lpfnWndProc = hackontrolRemoteProcedure;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH) COLOR_WINDOW;
	windowClass.lpszClassName = HACKONTROL_REMOTE;

	if(!RegisterClassExW(&windowClass)) {
		KHWin32DialogErrorW(GetLastError(), L"RegisterClassExW");
		return 1;
	}

	HWND window = CreateWindowExW(0, HACKONTROL_REMOTE, L"Hackontrol Remote", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 400, 400, NULL, NULL, instance, NULL);
	int returnValue = 1;

	if(!window) {
		KHWin32DialogErrorW(GetLastError(), L"CreateWindowExW");
		goto unregisterWindowClass;
	}

	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	returnValue = 0;
unregisterWindowClass:
	if(!UnregisterClassW(HACKONTROL_REMOTE, instance)) {
		KHWin32DialogErrorW(GetLastError(), L"UnregisterClassW");
		return 1;
	}

	return 0;
}
