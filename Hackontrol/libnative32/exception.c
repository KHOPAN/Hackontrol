#include <khopanjava.h>
#include <khopanstring.h>
#include "exception.h"

static BOOL isSpaceW(WCHAR character) {
	return character == L' ' || character == L'\t' || character == L'\r' || character == L'\n';
}

static LPWSTR trimWhitespaceW(LPWSTR text) {
	LPWSTR end;
	while(isSpaceW(*text)) text++;

	if(*text == 0) {
		return text;
	}

	end = text + wcslen(text) - 1;
	while(end > text && isSpaceW(*end)) end--;
	end[1] = 0;
	return text;
}

static void internalWin32Error(JNIEnv* const environment, const LPSTR errorClass, const LPWSTR functionName) {
	DWORD errorCode = GetLastError();
	LPWSTR messageBuffer = NULL;

	if(!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &messageBuffer, 0, NULL)) {
		KHJavaThrowInternalErrorW(environment, L"FormatMessageW() failed to format the error message");
		return;
	}

	LPWSTR message = KHFormatMessageW(L"%ws() %ws (Error code: %u)", functionName, trimWhitespaceW(messageBuffer), errorCode);
	LocalFree(messageBuffer);

	if(!message) {
		KHJavaThrowInternalErrorW(environment, L"KHFormatMessageW() failed to format the error message");
		return;
	}

	KHJavaThrowW(environment, errorClass, message);
	LocalFree(message);
}

void HackontrolThrowWin32Error(JNIEnv* const environment, const LPWSTR functionName) {
	internalWin32Error(environment, "com/khopan/hackontrol/exception/Win32Error", functionName);
}
