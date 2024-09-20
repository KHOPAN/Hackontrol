#include <libkhopan.h>

#define CLASS_NAME L"SimpleWindowClass"

static LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
	switch(message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT: {
		PAINTSTRUCT paintStruct;
		HDC context = BeginPaint(window, &paintStruct);
		HBRUSH brush = GetStockObject(DC_BRUSH);
		RECT bounds;
		GetClientRect(window, &bounds);
		SetDCBrushColor(context, 0x1E1E1E);
		FillRect(context, &bounds, brush);
		EndPaint(window, &paintStruct);
		return 0;
	}
	}

	return DefWindowProcW(window, message, wparam, lparam);
}

__declspec(dllexport) void __stdcall SimpleWindow(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	WNDCLASSW windowClass = {0};
	//windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = windowProcedure;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.lpszClassName = CLASS_NAME;

	if(!RegisterClassW(&windowClass)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"RegisterClassW");
		return;
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int windowSize = screenHeight / 2;

	if(!CreateWindowExW(0L, CLASS_NAME, L"Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, (screenWidth - windowSize) / 2, (screenHeight - windowSize) / 2, windowSize, windowSize, NULL, NULL, instance, NULL)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"CreateWindowExW");
		return;
	}

	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	if(!UnregisterClassW(CLASS_NAME, instance)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"UnregisterClassW");
	}
}
