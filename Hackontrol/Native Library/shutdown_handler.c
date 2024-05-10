#include <khopanjava.h>
#include "shutdown_handler.h"
#include "initialize.h"

#define CLASS_NAME L"HackontrolDummyWindow"

static JavaVM* globalVirtualMachine;

static DWORD WINAPI ShutdownHandlerThread(_In_ LPVOID parameter);
static LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

void RegisterShutdownHandler(JNIEnv* environment, JavaVM* virtualMachine) {
	globalVirtualMachine = virtualMachine;

	if(!CreateThread(NULL, 0, ShutdownHandlerThread, NULL, 0, NULL)) {
		KHJavaWin32ErrorW(environment, GetLastError(), L"CreateThread");
	}
}

static DWORD WINAPI ShutdownHandlerThread(_In_ LPVOID parameter) {
	HINSTANCE instance = GetProgramInstance();
	JavaVMAttachArgs arguments = {0};
	arguments.version = JNI_VERSION_21;
	arguments.name = NULL;
	arguments.group = NULL;
	JNIEnv* environment = NULL;

	if((*globalVirtualMachine)->AttachCurrentThread(globalVirtualMachine, (void**) &environment, &arguments)) {
		return 1;
	}

	WNDCLASSW windowClass = {0};
	windowClass.style = CS_VREDRAW | CS_HREDRAW;
	windowClass.lpfnWndProc = WindowProcedure;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH) COLOR_WINDOW;
	windowClass.lpszClassName = CLASS_NAME;

	if(!RegisterClassW(&windowClass)) {
		KHJavaWin32ErrorW(environment, GetLastError(), L"RegisterClassW");
		return 1;
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int windowWidth = (int) (screenWidth * 0.439238653);
	int windowHeight = (int) (screenHeight * 0.520833333);
	int windowX = (screenWidth - windowWidth) / 2;
	int windowY = (screenHeight - windowHeight) / 2;
	HWND window = CreateWindowExW(0L, CLASS_NAME, L"Always On Top", WS_OVERLAPPEDWINDOW, windowX, windowY, windowWidth, windowHeight, NULL, NULL, instance, NULL);

	if(!window) {
		KHJavaWin32ErrorW(environment, GetLastError(), L"CreateWindowExW");
		return 1;
	}

	if(!SetWindowPos(window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE)) {
		KHJavaWin32ErrorW(environment, GetLastError(), L"SetWindowPos");
		return 1;
	}

	ShowWindow(window, SW_NORMAL);
	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	if(!UnregisterClassW(CLASS_NAME, instance)) {
		KHJavaWin32ErrorW(environment, GetLastError(), L"UnregisterClassW");
		return 1;
	}

	return 0;
}

static LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
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
		SetDCBrushColor(context, 0x0000FF);
		RECT rectangle;
		GetClientRect(window, &rectangle);
		FillRect(context, &rectangle, brush);
		EndPaint(window, &paintStruct);
		return 0;
	}
	}

	return DefWindowProcW(window, message, wparam, lparam);
}
