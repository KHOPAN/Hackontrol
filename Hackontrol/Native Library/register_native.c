#include "register_native.h"
#include "native.h"

static JNINativeMethod HackontrolNativeMethods[] = {
	{"sleep",             "()Ljava/lang/String;",                     (void*) &NativeLibrary_sleep},
	{"hibernate",         "()Ljava/lang/String;",                     (void*) &NativeLibrary_hibernate},
	{"restart",           "()Ljava/lang/String;",                     (void*) &NativeLibrary_restart},
	{"shutdown",          "()Ljava/lang/String;",                     (void*) &NativeLibrary_shutdown},
	{"critical",          "(Z)Z",                                     (void*) &NativeLibrary_critical},
	{"dialog",            "(Ljava/lang/String;Ljava/lang/String;I)I", (void*) &NativeLibrary_dialog},
	{"listProcess",       "()[Lcom/khopan/hackontrol/ProcessEntry;",  (void*) &NativeLibrary_listProcess},
	{"currentIdentifier", "()I",                                      (void*) &Native_currentIdentifier},
	{"terminate",         "(I)Z",                                     (void*) &Native_terminate},
	{"hasUIAccess",       "()Z",                                      (void*) &Native_hasUIAccess},
	{"volume",            "(F)Z",                                     (void*) &Native_setVolume},
	{"volume",            "()F",                                      (void*) &Native_getVolume},
	{"mute",              "(Z)Z",                                     (void*) &Native_setMute},
	{"mute",              "()Z",                                      (void*) &Native_getMute},
	{"freeze",            "(Z)V",                                     (void*) &Native_freeze}
};

void RegisterHackontrolNative(JNIEnv* environment) {
	jclass nativeLibraryClass = (*environment)->FindClass(environment, "com/khopan/hackontrol/NativeLibrary");

	if(!nativeLibraryClass) {
		return;
	}

	(*environment)->RegisterNatives(environment, nativeLibraryClass, HackontrolNativeMethods, sizeof(HackontrolNativeMethods) / sizeof(HackontrolNativeMethods[0]));
}
