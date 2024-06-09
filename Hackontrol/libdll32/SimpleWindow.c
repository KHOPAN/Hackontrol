#include <khopanwin32.h>

#define CLASS_NAME L"SimpleWindowClass"

static LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
static void PaintWindow(HWND window);

__declspec(dllexport) void __stdcall SimpleWindow(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	WNDCLASSW windowClass = {0};
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProcedure;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.lpszClassName = CLASS_NAME;

	if(!RegisterClassW(&windowClass)) {
		KHWin32DialogErrorW(GetLastError(), L"RegisterClassW");
		return;
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int windowSize = screenHeight / 2;
	int windowX = (screenWidth - windowSize) / 2;
	int windowY = (screenHeight - windowSize) / 2;

	if(!CreateWindowExW(0L, CLASS_NAME, L"Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, windowX, windowY, windowSize, windowSize, NULL, NULL, instance, NULL)) {
		KHWin32DialogErrorW(GetLastError(), L"CreateWindowExW");
		return;
	}

	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	if(!UnregisterClassW(CLASS_NAME, instance)) {
		KHWin32DialogErrorW(GetLastError(), L"UnregisterClassW");
	}
}

static LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
	switch(message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
		PaintWindow(window);
		return 0;
	}

	return DefWindowProcW(window, message, wparam, lparam);
}

static void PaintWindow(HWND window) {
	PAINTSTRUCT paintStruct;
	HDC context = BeginPaint(window, &paintStruct);
	HBRUSH brush = GetStockObject(DC_BRUSH);
	SetDCBrushColor(context, 0x1E1E1E);
	RECT rectangle;
	GetClientRect(window, &rectangle);
	FillRect(context, &rectangle, brush);
	EndPaint(window, &paintStruct);
}
