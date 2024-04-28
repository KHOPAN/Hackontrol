#include "console.h"
#include "power.h"

jint NativeLibrary_sleep(JNIEnv* environment, jclass nativeLibraryClass) {
	System_out_println(environment, "Native: Sleep");
	return 0;
}

jint NativeLibrary_hibernate(JNIEnv* environment, jclass nativeLibraryClass) {
	System_out_println(environment, "Native: Hibernate");
	return 0;
}

jint NativeLibrary_restart(JNIEnv* environment, jclass nativeLibraryClass) {
	System_out_println(environment, "Native: Restart");
	return 0;
}

jint NativeLibrary_shutdown(JNIEnv* environment, jclass nativeLibraryClass) {
	System_out_println(environment, "Native: Shutdown");
	return 0;
}
