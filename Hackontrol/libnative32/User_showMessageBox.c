#include <khopanjava.h>
#include "User.h"

#define SAFE_FREE(x) if(x)LocalFree(x)

jint User_showMessageBox(JNIEnv* const environment, const jclass class, const jstring title, const jstring content, const jint flags) {
	LPWSTR titleNative = KHJavaToNativeStringW(environment, title);
	LPWSTR contentNative = KHJavaToNativeStringW(environment, content);
	jint response = MessageBoxW(NULL, contentNative, titleNative, flags);
	SAFE_FREE(titleNative);
	SAFE_FREE(contentNative);
	return response;
}
