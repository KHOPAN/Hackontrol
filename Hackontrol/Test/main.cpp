#include <stdio.h>
#include <d2d1.h>

#define CLASS_NAME L"TestWindowClass"

static ID2D1HwndRenderTarget* target;
static ID2D1SolidColorBrush* brush;

extern "C" {
	static LRESULT CALLBACK procedure(_In_ const HWND window, _In_ const UINT message, _In_ const WPARAM wparam, _In_ const LPARAM lparam) {
		PAINTSTRUCT paintStruct;

		switch(message) {
		case WM_CLOSE:
			DestroyWindow(window);
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_PAINT:
			BeginPaint(window, &paintStruct);
			target->BeginDraw();
			target->Clear();
			target->FillRectangle(D2D1::RectF(100.0f, 100.0f, 200.0f, 200.0f), brush);
			target->EndDraw();
			EndPaint(window, &paintStruct);
			return 0;
		}

		return DefWindowProcW(window, message, wparam, lparam);
	}
}

int main(const int argc, const char* const argv) {
	HINSTANCE instance = GetModuleHandleW(NULL);
	WNDCLASSW windowClass = {0};
	windowClass.lpfnWndProc = procedure;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.lpszClassName = CLASS_NAME;

	if(!RegisterClassW(&windowClass)) {
		printf("RegisterClassW() failed\n");
		return 1;
	}

	HWND window = CreateWindowExW(0L, CLASS_NAME, L"Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 600, 400, NULL, NULL, instance, NULL);
	int codeExit = 1;

	if(!window) {
		printf("CreateWindowExW() failed\n");
		goto unregisterClass;
	}

	ID2D1Factory* factory;

	if(FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory))) {
		printf("D2D1CreateFactory() failed\n");
		goto destroyWindow;
	}

	RECT bounds;
	GetClientRect(window, &bounds);

	if(FAILED(factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(window, D2D1::SizeU(bounds.right - bounds.left, bounds.bottom - bounds.top)), &target))) {
		printf("ID2D1Factory::CreateHwndRenderTarget() failed\n");
		goto releaseFactory;
	}

	if(FAILED(target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Aqua), &brush))) {
		printf("ID2D1HwndRenderTarget::CreateSolidColorBrush() failed\n");
		goto releaseTarget;
	}

	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	codeExit = 0;
	brush->Release();
releaseTarget:
	target->Release();
releaseFactory:
	factory->Release();
destroyWindow:
	DestroyWindow(window);
unregisterClass:
	UnregisterClassW(CLASS_NAME, instance);
	return codeExit;
}
