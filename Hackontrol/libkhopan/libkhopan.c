#include "libkhopan.h"

#define SAFECALL(x) {DWORD internalError=GetLastError();x;SetLastError(internalError);}

BOOL KHOPANEnablePrivilege(const LPWSTR privilege) {
	if(!privilege) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	HANDLE token;

	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
		return FALSE;
	}

	LUID identifier;

	if(!LookupPrivilegeValueW(NULL, privilege, &identifier)) {
		SAFECALL(CloseHandle(token));
		return FALSE;
	}

	TOKEN_PRIVILEGES privileges = {0};
	privileges.PrivilegeCount = 1;
	privileges.Privileges[0].Luid = identifier;
	privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	identifier.LowPart = AdjustTokenPrivileges(token, FALSE, &privileges, sizeof(privileges), NULL, NULL);
	SAFECALL(CloseHandle(token));

	if(!identifier.LowPart) {
		return FALSE;
	}

	SetLastError(ERROR_SUCCESS);
	return TRUE;
}

LPWSTR KHOPANFileGetCmd() {
	LPWSTR folderWindows = KHOPANFolderGetWindows();

	if(!folderWindows) {
		return NULL;
	}

	LPWSTR fileCommandPrompt = KHOPANFormatMessage(L"%ws\\" FOLDER_SYSTEM32 L"\\" FILE_CMD, folderWindows);
	SAFECALL(LocalFree(folderWindows));
	return fileCommandPrompt;
}

LPWSTR KHOPANFileGetRundll32() {
	LPWSTR folderWindows = KHOPANFolderGetWindows();

	if(!folderWindows) {
		return NULL;
	}

	LPWSTR fileRundll32 = KHOPANFormatMessage(L"%ws\\" FOLDER_SYSTEM32 L"\\" FILE_RUNDLL32, folderWindows);
	SAFECALL(LocalFree(folderWindows));
	return fileRundll32;
}

LPWSTR KHOPANFolderGetWindows() {
	UINT size = GetSystemWindowsDirectoryW(NULL, 0);

	if(!size) {
		return NULL;
	}

	LPWSTR buffer = LocalAlloc(LMEM_FIXED, size * sizeof(WCHAR));

	if(!buffer) {
		return NULL;
	}

	if(!GetSystemWindowsDirectoryW(buffer, size)) {
		SAFECALL(LocalFree(buffer));
		return NULL;
	}

	SetLastError(ERROR_SUCCESS);
	return buffer;
}

LPWSTR KHOPANFormatMessage(const LPWSTR format, ...) {
	if(!format) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return NULL;
	}

	va_list list;
	va_start(list, format);
	int length = _vscwprintf(format, list);
	LPWSTR buffer = NULL;

	if(length == -1) {
		SetLastError(ERROR_INVALID_PARAMETER);
		goto functionExit;
	}

	buffer = LocalAlloc(LMEM_FIXED, (((size_t) length) + 1) * sizeof(WCHAR));

	if(!buffer) {
		goto functionExit;
	}

	if(vswprintf_s(buffer, ((size_t) length) + 1, format, list) == -1) {
		LocalFree(buffer);
		buffer = NULL;
		SetLastError(ERROR_INVALID_PARAMETER);
		goto functionExit;
	}

	SetLastError(ERROR_SUCCESS);
functionExit:
	va_end(list);
	return buffer;
}

LPWSTR KHOPANInternalGetErrorMessage(const DWORD code, const LPCWSTR function, const BOOL win32) {
	LPWSTR buffer;

	if(!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | (win32 ? 0 : FORMAT_MESSAGE_FROM_HMODULE), win32 ? NULL : LoadLibraryW(L"ntdll.dll"), code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &buffer, 0, NULL)) {
		return function ? KHOPANFormatMessage(L"%ws() error occurred. Error code: %lu", function, code) : KHOPANFormatMessage(L"Error occurred. Error code: %lu", code);
	}

	LPWSTR message = function ? KHOPANFormatMessage(L"%ws() error occurred. Error code: %lu Message:\n%ws", function, code, buffer) : KHOPANFormatMessage(L"Error occurred. Error code: %lu Message:\n%ws", code, buffer);
	SAFECALL(LocalFree(buffer));
	return message;
}
