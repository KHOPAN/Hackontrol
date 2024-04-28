#include <jni.h>
#include "console.h"
#include "power.h"

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

	System_out_println(environment, "Hello world, from Native Library!");
	jclass nativeLibraryClass = (*environment)->FindClass(environment, "com/khopan/hackontrol/NativeLibrary");
	
	if(!nativeLibraryClass) {
		System_err_println(environment, "Class 'com.khopan.hackontrol.NativeLibrary' not found");
		goto nativeBindingFail;
	}

	jint result = (*environment)->RegisterNatives(environment, nativeLibraryClass, HackontrolNativeMethods, sizeof(HackontrolNativeMethods) / sizeof(HackontrolNativeMethods[0]));

	if(result < 0) {
		System_err_println(environment, "Failed to register native methods for class 'com.khopan.hackontrol.NativeLibrary'");
	}

nativeBindingFail:
	/*BOOL result = ProtectProcess();

	if(!result) {
		System_err_println(environment, "Process protection has failed!");
	}*/

	return JNI_VERSION_21;
}
