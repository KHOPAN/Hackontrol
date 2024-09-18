#include "khopanstring.h"
#include "khopancurl.h"

#define FORMATA "%s() error ocurred. Error code: %u Message:\n%s"
#define FORMATW L"%ws() error ocurred. Error code: %u Message:\n%S"

static void* allocate(const void* data, size_t size) {
	BYTE* buffer = LocalAlloc(LMEM_FIXED, size);

	if(!buffer) {
		return NULL;
	}

	memcpy_s(buffer, size, data, size);
	return buffer;
}

LPSTR KHCURLGetErrorMessageA(CURLcode errorCode, const LPSTR functionName) {
	const char* errorMessage = curl_easy_strerror(errorCode);
	LPSTR message = KHFormatMessageA(FORMATA, functionName, errorCode, errorMessage);

	if(!message) {
		return allocate(errorMessage, (strlen(errorMessage) + 1) * sizeof(CHAR));
	}

	return message;
}

LPWSTR KHCURLGetErrorMessageW(CURLcode errorCode, const LPWSTR functionName) {
	const char* errorMessage = curl_easy_strerror(errorCode);
	LPWSTR message = KHFormatMessageW(FORMATW, functionName, errorCode, errorMessage);

	if(!message) {
		return allocate(L"Failed to format the error message", 35 * sizeof(WCHAR));
	}

	return message;
}

void KHCURLDialogErrorA(CURLcode errorCode, const LPSTR functionName) {
	LPSTR message = KHCURLGetErrorMessageA(errorCode, functionName);
	MessageBoxA(NULL, message, "Error", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
	LocalFree(message);
}

void KHCURLDialogErrorW(CURLcode errorCode, const LPWSTR functionName) {
	LPWSTR message = KHCURLGetErrorMessageW(errorCode, functionName);
	MessageBoxW(NULL, message, L"Error", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
	LocalFree(message);
}

void KHCURLConsoleErrorA(CURLcode errorCode, const LPSTR functionName) {
	LPSTR message = KHCURLGetErrorMessageA(errorCode, functionName);
	printf("%s\n", message);
	LocalFree(message);
}

void KHCURLConsoleErrorW(CURLcode errorCode, const LPWSTR functionName) {
	LPWSTR message = KHCURLGetErrorMessageW(errorCode, functionName);
	printf("%ws\n", message);
	LocalFree(message);
}
