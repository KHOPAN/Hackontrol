#include "libkhopan.h"

LPWSTR KHOPANInternalGetErrorMessage(const DWORD code, const LPCWSTR function, const BOOL win32) {
	LPWSTR buffer;

	if(!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | (win32 ? 0 : FORMAT_MESSAGE_FROM_HMODULE), win32 ? NULL : LoadLibraryW(L"ntdll.dll"), code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &buffer, 0, NULL)) {
		return function ? KHOPANFormatMessage(L"%ws() error occurred. Error code: %lu", function, code) : KHOPANFormatMessage(L"Error occurred. Error code: %lu", code);
	}

	LPWSTR message = function ? KHOPANFormatMessage(L"%ws() error occurred. Error code: %lu Message:\n%ws", function, code, buffer) : KHOPANFormatMessage(L"Error occurred. Error code: %lu Message:\n%ws", code, buffer);
	LocalFree(buffer);
	return message;
}

LPWSTR KHOPANFormatMessage(const LPWSTR format, ...) {
	va_list list;
	va_start(list, format);
	int length = _vscwprintf(format, list);
	LPWSTR buffer = NULL;

	if(length == -1) {
		goto functionExit;
	}

	buffer = LocalAlloc(LMEM_FIXED, (((size_t) length) + 1) * sizeof(WCHAR));

	if(!buffer) {
		goto functionExit;
	}

	if(vswprintf_s(buffer, ((size_t) length) + 1, format, list) == -1) {
		LocalFree(buffer);
		buffer = NULL;
	}
functionExit:
	va_end(list);
	return buffer;
}

LPWSTR KHOPANDirectoryGetWindows() {
	UINT size = GetSystemWindowsDirectoryW(NULL,0);

	if(!size) {
		return NULL;
	}

	LPWSTR buffer = LocalAlloc(LMEM_FIXED, size * sizeof(WCHAR));

	if(!buffer) {
		return NULL;
	}

	if(!GetSystemWindowsDirectoryW(buffer, size)) {
		LocalFree(buffer);
		return NULL;
	}

	return buffer;
}
