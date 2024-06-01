#include <Windows.h>
#include <khopanjava.h>
#include "exception.h"
#include "Kernel.h"
#include "instance.h"

#define CLASS_NAME L"IFreezerClass"

static BOOL globalInitialized;
static HBITMAP globalBitmap;
static HWND globalWindow;

static LRESULT CALLBACK freezerProcedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
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

static DWORD WINAPI freezerThread(_In_ LPVOID parameter) {
	JavaVM* virtualMachine = (JavaVM*) parameter;

	if(!virtualMachine) {
		return 1;
	}

	JavaVMAttachArgs arguments = {0};
	arguments.version = JNI_VERSION_21;
	arguments.name = NULL;
	arguments.group = NULL;
	JNIEnv* environment = NULL;

	if((*virtualMachine)->AttachCurrentThread(virtualMachine, (void**) &environment, &arguments) != JNI_OK) {
		return 1;
	}

	HINSTANCE instance = GetProgramInstance();
	WNDCLASSW windowClass = {0};
	windowClass.style = CS_VREDRAW | CS_HREDRAW;
	windowClass.lpfnWndProc = freezerProcedure;
	windowClass.hInstance = instance;
	windowClass.hCursor = NULL;
	windowClass.lpszClassName = CLASS_NAME;
	DWORD returnValue = 1;

	if(!RegisterClassW(&windowClass)) {
		HackontrolThrowWin32Error(environment, L"RegisterClassW");
		goto detachThread;
	}

	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);
	HWND window = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, CLASS_NAME, NULL, WS_POPUP, 0, 0, width, height, NULL, NULL, instance, NULL);

	if(!window) {
		HackontrolThrowWin32Error(environment, L"CreateWindowExW");
		goto detachThread;
	}

	globalWindow = window;
	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	if(!UnregisterClassW(CLASS_NAME, instance)) {
		HackontrolThrowWin32Error(environment, L"UnregisterClassW");
		goto detachThread;
	}

	returnValue = 0;
detachThread:
	if((*virtualMachine)->DetachCurrentThread(virtualMachine) != JNI_OK) {
		SetLastError(ERROR_FUNCTION_FAILED);
		HackontrolThrowWin32Error(environment, L"JavaVM::DetachCurrentThread");
		return 1;
	}

	return returnValue;
}

static BOOL captureScreen(JNIEnv* const environment) {
	HDC screenContext = GetDC(NULL);

	if(!screenContext) {
		SetLastError(ERROR_FUNCTION_FAILED);
		HackontrolThrowWin32Error(environment, L"GetDC");
		return FALSE;
	}

	HDC memoryContext = CreateCompatibleDC(screenContext);
	BOOL returnValue = FALSE;

	if(!memoryContext) {
		SetLastError(ERROR_FUNCTION_FAILED);
		HackontrolThrowWin32Error(environment, L"CreateCompatibleDC");
		goto deleteScreenContext;
	}

	int width = GetDeviceCaps(screenContext, HORZRES);
	int height = GetDeviceCaps(screenContext, VERTRES);
	HBITMAP bitmap = CreateCompatibleBitmap(screenContext, width, height);

	if(!memoryContext) {
		SetLastError(ERROR_FUNCTION_FAILED);
		HackontrolThrowWin32Error(environment, L"CreateCompatibleBitmap");
		goto deleteMemoryContext;
	}

	HBITMAP oldBitmap = SelectObject(memoryContext, bitmap);

	if(!oldBitmap || oldBitmap == HGDI_ERROR) {
		SetLastError(ERROR_FUNCTION_FAILED);
		HackontrolThrowWin32Error(environment, L"SelectObject");
		goto deleteMemoryContext;
	}

	if(!BitBlt(memoryContext, 0, 0, width, height, screenContext, 0, 0, SRCCOPY)) {
		HackontrolThrowWin32Error(environment, L"BitBlt");
		goto deleteMemoryContext;
	}

	bitmap = SelectObject(memoryContext, oldBitmap);

	if(!bitmap || bitmap == HGDI_ERROR) {
		SetLastError(ERROR_FUNCTION_FAILED);
		HackontrolThrowWin32Error(environment, L"SelectObject");
		goto deleteMemoryContext;
	}

	globalBitmap = bitmap;
	returnValue = TRUE;
deleteMemoryContext:
	DeleteDC(memoryContext);
deleteScreenContext:
	DeleteDC(screenContext);
	return returnValue;
}

void Kernel_setFreeze(JNIEnv* const environment, const jclass class, const jboolean freeze) {
	if(!globalInitialized) {
		globalInitialized = TRUE;
		JavaVM* virtualMachine;

		if((*environment)->GetJavaVM(environment, &virtualMachine)) {
			SetLastError(ERROR_FUNCTION_FAILED);
			HackontrolThrowWin32Error(environment, L"JNIEnv::GetJavaVM");
			return;
		}

		if(!CreateThread(NULL, 0, freezerThread, virtualMachine, 0, NULL)) {
			HackontrolThrowWin32Error(environment, L"CreateThread");
			return;
		}
	}

	if(freeze && !captureScreen(environment)) {
		return;
	}

	ShowWindow(globalWindow, freeze ? SW_SHOW : SW_HIDE);
}
