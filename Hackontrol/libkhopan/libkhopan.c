#include "libkhopan.h"

LPWSTR KHOPANInternalGetErrorMessage(const DWORD code, const LPCWSTR function, const BOOL win32) {
	LPWSTR buffer;

	if(!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | (win32 ? 0 : FORMAT_MESSAGE_FROM_HMODULE), win32 ? NULL : LoadLibraryW(L"ntdll.dll"), code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &buffer, 0, NULL)) {
		return function ? KHOPANFormatMessage(L"%ws() error occurred. Error code: %lu", function, code) : KHOPANFormatMessage(L"Error occurred. Error code: %lu", code);
	}

	LPWSTR message = function ? KHOPANFormatMessage(L"%ws() error occurred. Error code: %lu Message:\n%ws", function, code, buffer) : KHOPANFormatMessage(L"Error occurred. Error code: %lu Message:\n%ws", code, buffer);
	DWORD error = GetLastError();
	LocalFree(buffer);
	SetLastError(error);
	return message;
}

LPWSTR KHOPANFormatMessage(const LPWSTR format, ...) {
	va_list list;
	va_start(list, format);
	int length = _vscwprintf(format, list);
	LPWSTR buffer = NULL;
	DWORD error = ERROR_SUCCESS;

	if(length == -1) {
		error = ERROR_INVALID_PARAMETER;
		goto functionExit;
	}

	buffer = LocalAlloc(LMEM_FIXED, (((size_t) length) + 1) * sizeof(WCHAR));

	if(!buffer) {
		error = GetLastError();
		goto functionExit;
	}

	if(vswprintf_s(buffer, ((size_t) length) + 1, format, list) == -1) {
		LocalFree(buffer);
		buffer = NULL;
		error = ERROR_INVALID_PARAMETER;
		goto functionExit;
	}
functionExit:
	va_end(list);
	SetLastError(error);
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

	DWORD error = ERROR_SUCCESS;

	if(!GetSystemWindowsDirectoryW(buffer, size)) {
		error = GetLastError();
		LocalFree(buffer);
		buffer = NULL;
		goto functionExit;
	}
functionExit:
	SetLastError(error);
	return buffer;
}

LPWSTR KHOPANFileGetRundll32() {
	LPWSTR folderWindows = KHOPANDirectoryGetWindows();

	if(!folderWindows) {
		return NULL;
	}

	LPWSTR fileRundll32 = KHOPANFormatMessage(L"%ws\\" FOLDER_SYSTEM32 L"\\" FILE_RUNDLL32, folderWindows);
	DWORD error = GetLastError();
	LocalFree(folderWindows);
	SetLastError(error);
	return fileRundll32;
}

LPWSTR KHOPANFileGetCmd() {
	LPWSTR folderWindows = KHOPANDirectoryGetWindows();

	if(!folderWindows) {
		return NULL;
	}

	LPWSTR fileCommandPrompt = KHOPANFormatMessage(L"%ws\\" FOLDER_SYSTEM32 L"\\" FILE_CMD, folderWindows);
	DWORD error = GetLastError();
	LocalFree(folderWindows);
	SetLastError(error);
	return fileCommandPrompt;
}
