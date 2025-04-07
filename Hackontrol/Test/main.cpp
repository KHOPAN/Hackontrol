#include <stdio.h>
#include <d2d1.h>

#define CLASS_NAME L"TestWindowClass"

static LARGE_INTEGER timeFrequency;
static ID2D1HwndRenderTarget* target;
static ID2D1SolidColorBrush* brush;
static LARGE_INTEGER startTime;

static inline double interpolate(double time) {
	return ((time *= 2.0) < 1.0 ? time * time * time : ((time -= 2.0) * time * time + 2.0)) * 0.5;
}

extern "C" {
	static LRESULT CALLBACK procedure(_In_ const HWND window, _In_ const UINT message, _In_ const WPARAM wparam, _In_ const LPARAM lparam) {
		PAINTSTRUCT paintStruct;
		RECT bounds;
		LARGE_INTEGER tick;
		double time;
		double x;

		switch(message) {
		case WM_CLOSE:
			DestroyWindow(window);
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_LBUTTONDOWN:
			QueryPerformanceCounter(&startTime);
			return 0;
		case WM_PAINT:
			BeginPaint(window, &paintStruct);
			target->BeginDraw();
			target->Clear();
			GetClientRect(window, &bounds);
			QueryPerformanceCounter(&tick);
			time = ((double) (tick.QuadPart - startTime.QuadPart)) / (((double) timeFrequency.QuadPart) * 0.5);
			time = interpolate(min(time, 1.0));
			time = min(max(time, 0.0), 1.0);
			x = (((double) bounds.right) - ((double) bounds.left) - 100.0) * time;
			target->FillRectangle(D2D1::RectF((FLOAT) x, 100.0f, (FLOAT) (x + 100.0), 200.0f), brush);
			target->EndDraw();
			EndPaint(window, &paintStruct);
			InvalidateRect(window, NULL, FALSE);
			return 0;
		case WM_SIZE:
			target->Resize(D2D1::SizeU(LOWORD(lparam), HIWORD(lparam)));
			return 0;
		}

		return DefWindowProcW(window, message, wparam, lparam);
	}
}

int main(const int argc, const char* const argv) {
	if(FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED))) {
		printf("CoInitializeEx() failed\n");
		return 1;
	}

	QueryPerformanceFrequency(&timeFrequency);
	HINSTANCE instance = GetModuleHandleW(NULL);
	WNDCLASSW windowClass = {0};
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = procedure;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.lpszClassName = CLASS_NAME;
	int codeExit = 1;

	if(!RegisterClassW(&windowClass)) {
		printf("RegisterClassW() failed\n");
		goto uninitialize;
	}

	HWND window;

	if(!(window = CreateWindowExW(0L, CLASS_NAME, L"Window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 600, 400, NULL, NULL, instance, NULL))) {
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

	ShowWindow(window, SW_NORMAL);
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
uninitialize:
	CoUninitialize();
	return codeExit;
}
