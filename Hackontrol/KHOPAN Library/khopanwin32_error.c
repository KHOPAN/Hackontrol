#include <stdio.h>
#include "khopanstring.h"
#include "khopanwin32.h"

#define FORMATA "%s() error ocurred. Error code: %u Message:\n%s"
#define FORMATW L"%ws() error ocurred. Error code: %u Message:\n%ws"

static void* allocate(void* data, size_t size) {
	BYTE* buffer = LocalAlloc(LMEM_FIXED, size);

	if(!buffer) {
		return NULL;
	}

	memcpy_s(buffer, size, data, size);
	return buffer;
}

LPSTR KHWin32GetErrorMessageA(DWORD errorCode, const LPSTR functionName) {
	LPSTR messageBuffer = NULL;
	DWORD size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR) &messageBuffer, 0, NULL);

	if(!size) {
		return allocate("Error while getting the error message text", 43 * sizeof(CHAR));
	}

	LPSTR message = KHFormatMessageA(FORMATA, functionName, errorCode, messageBuffer);

	if(!message) {
		return messageBuffer;
	}

	return message;
}

LPWSTR KHWin32GetErrorMessageW(DWORD errorCode, const LPWSTR functionName) {
	LPWSTR messageBuffer = NULL;
	DWORD size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &messageBuffer, 0, NULL);

	if(!size) {
		return allocate(L"Error while getting the error message text", 43 * sizeof(WCHAR));
	}

	LPWSTR message = KHFormatMessageW(FORMATW, functionName, errorCode, messageBuffer);

	if(!message) {
		return messageBuffer;
	}

	return message;
}

void KHWin32DialogErrorA(DWORD errorCode, const LPSTR functionName) {
	LPSTR message = KHWin32GetErrorMessageA(errorCode, functionName);
	MessageBoxA(NULL, message, "Error", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
	LocalFree(message);
}

void KHWin32DialogErrorW(DWORD errorCode, const LPWSTR functionName) {
	LPWSTR message = KHWin32GetErrorMessageW(errorCode, functionName);
	MessageBoxW(NULL, message, L"Error", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
	LocalFree(message);
}

void KHWin32ConsoleErrorA(DWORD errorCode, const LPSTR functionName) {
	LPSTR message = KHWin32GetErrorMessageA(errorCode, functionName);
	printf("%s\n", message);
	LocalFree(message);
}

void KHWin32ConsoleErrorW(DWORD errorCode, const LPWSTR functionName) {
	LPWSTR message = KHWin32GetErrorMessageW(errorCode, functionName);
	printf("%ws\n", message);
	LocalFree(message);
}

DWORD KHWin32DecodeHRESULTError(HRESULT result) {
	if((result & 0xFFFF0000) == MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, 0)) {
		return HRESULT_CODE(result);
	}

	if(result == S_OK) {
		return ERROR_SUCCESS;
	}

	return ERROR_CAN_NOT_COMPLETE;
}
