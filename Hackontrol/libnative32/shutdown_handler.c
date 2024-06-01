#include <khopanwin32.h>
#include <khopanjava.h>
#include "shutdown_handler.h"
#include "Kernel.h"
#include "instance.h"

#define CLASS_NAME L"HackontrolDummyWindow"

static JavaVM* globalVirtualMachine;
static JNIEnv* globalEnvironment;

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

	globalEnvironment = environment;
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

	HWND window = CreateWindowExW(0L, CLASS_NAME, NULL, WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, NULL, NULL, instance, NULL);

	if(!window) {
		KHJavaWin32ErrorW(environment, GetLastError(), L"CreateWindowExW");
		return 1;
	}

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
	case WM_QUERYENDSESSION:
		if(lparam != 0 && !(lparam & ENDSESSION_CRITICAL)) {
			return TRUE;
		}

		Kernel_setProcessCritical(globalEnvironment, NULL, FALSE);
		return TRUE;
	}

	return DefWindowProcW(window, message, wparam, lparam);
}
