#include <khopanjni.h>
#include "register_native.h"
#include "native_power.h"
#include "native_critical.h"
#include "native_dialog.h"

static JNINativeMethod HackontrolNativeMethods[] = {
	{"sleep",     "()Ljava/lang/String;",                     (void*) &NativeLibrary_sleep},
	{"hibernate", "()Ljava/lang/String;",                     (void*) &NativeLibrary_hibernate},
	{"restart",   "()Ljava/lang/String;",                     (void*) &NativeLibrary_restart},
	{"shutdown",  "()Ljava/lang/String;",                     (void*) &NativeLibrary_shutdown},
	{"critical",  "(Z)Z",                                     (void*) &NativeLibrary_critical},
	{"dialog",    "(Ljava/lang/String;Ljava/lang/String;I)I", (void*) &NativeLibrary_dialog}
};

void RegisterHackontrolNative(JNIEnv* environment) {
	jclass nativeLibraryClass = (*environment)->FindClass(environment, "com/khopan/hackontrol/NativeLibrary");

	if(!nativeLibraryClass) {
		KHStandardErrorW(environment, L"Class 'com.khopan.hackontrol.NativeLibrary' not found");
		return;
	}

	if((*environment)->RegisterNatives(environment, nativeLibraryClass, HackontrolNativeMethods, sizeof(HackontrolNativeMethods) / sizeof(HackontrolNativeMethods[0])) < 0) {
		KHStandardErrorW(environment, L"Failed to register native methods for class 'com.khopan.hackontrol.NativeLibrary'");
	}
}
