#include "khopanwin32.h"
#include "khopanstring.h"

#define ERROR_FORMAT          L"%ws() error ocurred in '%ws' Line: %u Error code: %u Message:\n%ws"
#define ERROR_FORMAT_FALLBACK L"%ws() error ocurred in '%ws' Line: %u Error code: %u"

LPSTR KHWin32GetWindowsDirectoryA() {
	UINT size = GetSystemWindowsDirectoryA(NULL, 0);

	if(!size) {
		return NULL;
	}

	LPSTR buffer = LocalAlloc(LMEM_FIXED, size);

	if(!buffer) {
		return NULL;
	}

	if(!GetSystemWindowsDirectoryA(buffer, size)) {
		LocalFree(buffer);
		return NULL;
	}

	return buffer;
}

LPWSTR KHWin32GetWindowsDirectoryW() {
	UINT size = GetSystemWindowsDirectoryW(NULL, 0);

	if(!size) {
		return NULL;
	}

	LPWSTR buffer = LocalAlloc(LMEM_FIXED, size);

	if(!buffer) {
		return NULL;
	}

	if(!GetSystemWindowsDirectoryW(buffer, size)) {
		LocalFree(buffer);
		return NULL;
	}

	return buffer;
}

LPWSTR KHInternal_ErrorMessage(const DWORD errorCode, const LPCWSTR functionName, const LPCWSTR fileName, const UINT lineNumber, const BOOL specialError) {
	LPWSTR buffer;

	if(!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | (specialError ? FORMAT_MESSAGE_FROM_HMODULE : 0), specialError ? LoadLibraryW(L"ntdll.dll") : NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &buffer, 0, NULL)) {
		return KHFormatMessageW(ERROR_FORMAT_FALLBACK, functionName, fileName, lineNumber, errorCode);
	}

	LPWSTR message = KHFormatMessageW(ERROR_FORMAT, functionName, fileName, lineNumber, errorCode, buffer);

	if(!message) {
		return buffer;
	}

	LocalFree(buffer);
	return message;
}
