#include "native_dialog.h"
#include <khopanjni.h>

jint NativeLibrary_dialog(JNIEnv* environment, jclass nativeLibraryClass, jstring title, jstring content, jint flags) {
	LPWSTR titleNative = KHJavaToNativeStringW(environment, title);
	LPWSTR contentNative = KHJavaToNativeStringW(environment, content);
	jint response = MessageBoxW(NULL, contentNative, titleNative, flags);

	if(titleNative) {
		LocalFree(titleNative);
	}

	if(contentNative) {
		LocalFree(contentNative);
	}

	return response;
}
