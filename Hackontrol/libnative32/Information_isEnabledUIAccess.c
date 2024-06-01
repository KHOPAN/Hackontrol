#include "exception.h"
#include "Information.h"

jboolean Information_isEnabledUIAccess(JNIEnv* const environment, const jclass class) {
	HANDLE token;

	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
		HackontrolThrowWin32Error(environment, L"OpenProcessToken");
		return FALSE;
	}

	DWORD access;
	DWORD returnLength;

	if(!GetTokenInformation(token, TokenUIAccess, &access, sizeof(access), &returnLength)) {
		HackontrolThrowWin32Error(environment, L"GetTokenInformation");
		CloseHandle(token);
		return FALSE;
	}

	return (jboolean) access;
}
