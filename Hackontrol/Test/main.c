#include <Windows.h>

#define CLASS_NAME L"WindowClass"

static int x;

static void paint(const HWND window) {
	PAINTSTRUCT paintStruct;
	HDC context = BeginPaint(window, &paintStruct);
	HBRUSH brush = GetStockObject(DC_BRUSH);
	SetDCBrushColor(context, 0x7F7F7F);
	RECT bounds;
	GetClientRect(window, &bounds);
	FillRect(context, &bounds, brush);
	//ExtTextOutW(context, 100, 100, 0, NULL, L"Hello, world!", 13, NULL);
	bounds.left = x;
	bounds.top = 100;
	bounds.right = bounds.left + 100;
	bounds.bottom = bounds.top + 100;
	SetDCBrushColor(context, 0x0000FF);
	FillRect(context, &bounds, brush);
	EndPaint(window, &paintStruct);
}

static LRESULT CALLBACK procedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	switch(message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
		paint(window);
		return 0;
	}

	return DefWindowProcW(window, message, wparam, lparam);
}

static void CALLBACK timerFunction(HWND window, UINT message, UINT_PTR identifier, DWORD whatIsThis) {
	x++;
	InvalidateRect(window, NULL, FALSE);
}

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE previousInstance, _In_ LPSTR command, _In_ int show) {
	WNDCLASSW windowClass = {0};
	windowClass.lpfnWndProc = procedure;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	windowClass.lpszClassName = CLASS_NAME;

	if(!RegisterClassW(&windowClass)) {
		return 1;
	}

	HWND window = CreateWindowExW(0L, CLASS_NAME, L"Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 600, 400, NULL, NULL, instance, NULL);
	int codeExit = 1;

	if(!window) {
		goto unregisterClass;
	}

	SetTimer(window, 1, 10, timerFunction);
	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	codeExit = 0;
unregisterClass:
	UnregisterClassW(CLASS_NAME, instance);
	return codeExit;
}
