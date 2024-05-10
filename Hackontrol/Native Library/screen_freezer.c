#include <khopanjava.h>
#include "screen_freezer.h"
#include "initialize.h"
#include "native.h"

#define CLASS_NAME L"HackontrolCoverWindow"

static JavaVM* globalVirtualMachine;
static HBITMAP globalBitmap;
static HWND globalWindow;

static DWORD WINAPI ScreenFreezerThread(_In_ LPVOID parameter);
static LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

void InitializeScreenFreezer(JNIEnv* environment, JavaVM* virtualMachine) {
	globalVirtualMachine = virtualMachine;

	if(!CreateThread(NULL, 0, ScreenFreezerThread, NULL, 0, NULL)) {
		KHJavaWin32ErrorW(environment, GetLastError(), L"CreateThread");
	}
}

void Native_freeze(JNIEnv* environment, jclass nativeLibraryClass, jboolean freeze) {
	if(freeze) {
		HDC screenContext = GetDC(NULL);
		HDC memoryContext = CreateCompatibleDC(screenContext);
		int width = GetDeviceCaps(screenContext, HORZRES);
		int height = GetDeviceCaps(screenContext, VERTRES);
		HBITMAP bitmap = CreateCompatibleBitmap(screenContext, width, height);
		HBITMAP oldBitmap = SelectObject(memoryContext, bitmap);
		BitBlt(memoryContext, 0, 0, width, height, screenContext, 0, 0, SRCCOPY);
		bitmap = SelectObject(memoryContext, oldBitmap);
		DeleteDC(memoryContext);
		DeleteDC(screenContext);
		globalBitmap = bitmap;
	}

	ShowWindow(globalWindow, freeze ? SW_SHOW : SW_HIDE);
}

static DWORD WINAPI ScreenFreezerThread(_In_ LPVOID parameter) {
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
	windowClass.hCursor = NULL;
	windowClass.lpszClassName = CLASS_NAME;

	if(!RegisterClassW(&windowClass)) {
		KHJavaWin32ErrorW(environment, GetLastError(), L"RegisterClassW");
		return 1;
	}

	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);
	HWND window = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, CLASS_NAME, NULL, WS_POPUP, 0, 0, width, height, NULL, NULL, instance, NULL);

	if(!window) {
		KHJavaWin32ErrorW(environment, GetLastError(), L"CreateWindowExW");
		return 1;
	}

	globalWindow = window;
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
	case WM_PAINT:
		if(!globalBitmap) {
			break;
		}

		PAINTSTRUCT paintStruct;
		HDC context = BeginPaint(window, &paintStruct);
		HDC memoryContext = CreateCompatibleDC(context);
		HBITMAP oldBitmap = SelectObject(memoryContext, globalBitmap);
		BITMAP bitmap;
		GetObjectW(globalBitmap, sizeof(bitmap), &bitmap);
		BitBlt(context, 0, 0, bitmap.bmWidth, bitmap.bmHeight, memoryContext, 0, 0, SRCCOPY);
		SelectObject(memoryContext, oldBitmap);
		DeleteDC(memoryContext);
		EndPaint(window, &paintStruct);
		return 0;
	case WM_SETCURSOR:
		SetCursor(NULL);
		return 0;
	}

	return DefWindowProcW(window, message, wparam, lparam);
}
