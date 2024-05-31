#include <Windows.h>
#include "exception.h"
#include "Information.h"

jstring Information_getUserName(JNIEnv* const environment, const jclass class) {
	DWORD length = 0;

	if(!GetUserNameW(NULL, &length) && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
		HackontrolThrowWin32Error(environment, L"GetUserNameW");
		return NULL;
	}

	if(length < 1) {
		SetLastError(ERROR_INSUFFICIENT_BUFFER);
		HackontrolThrowWin32Error(environment, L"Information_getUserName");
		return NULL;
	}

	LPWSTR buffer = LocalAlloc(LMEM_FIXED, length * sizeof(WCHAR));

	if(!buffer) {
		HackontrolThrowWin32Error(environment, L"LocalAlloc");
		return NULL;
	}

	if(!GetUserNameW(buffer, &length)) {
		HackontrolThrowWin32Error(environment, L"GetUserNameW");
		return NULL;
	}

	jstring string = (*environment)->NewString(environment, buffer, length - 1);
	LocalFree(buffer);
	return string;
}
