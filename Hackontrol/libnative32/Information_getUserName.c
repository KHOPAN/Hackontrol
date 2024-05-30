#include <Windows.h>
#include "Information.h"

jstring Information_getUserName(JNIEnv* const environment, jclass const class) {
	DWORD length = 0;

	if(!GetUserNameW(NULL, &length) && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
		return NULL;
	}

	if(length < 1) {
		return NULL;
	}

	LPWSTR buffer = LocalAlloc(LMEM_FIXED, length * sizeof(WCHAR));

	if(!buffer) {
		return NULL;
	}

	if(!GetUserNameW(buffer, &length)) {
		return NULL;
	}

	jstring string = (*environment)->NewString(environment, buffer, length - 1);
	LocalFree(buffer);
	return string;
}
