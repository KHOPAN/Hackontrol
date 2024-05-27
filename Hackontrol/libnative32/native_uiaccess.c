#include <khopanjava.h>
#include "native.h"

jboolean Native_hasUIAccess(JNIEnv* environment, jclass nativeLibraryClass) {
	HANDLE token;

	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
		KHJavaWin32ErrorW(environment, GetLastError(), L"OpenProcessToken");
		return FALSE;
	}

	BOOL returnValue = FALSE;
	DWORD access;
	DWORD returnLength;

	if(!GetTokenInformation(token, TokenUIAccess, &access, sizeof(access), &returnLength)) {
		KHJavaWin32ErrorW(environment, GetLastError(), L"GetTokenInformation");
		CloseHandle(token);
		return FALSE;
	}

	return (jboolean) access;
}
