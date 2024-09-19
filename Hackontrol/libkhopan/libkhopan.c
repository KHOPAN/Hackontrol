#include "libkhopan.h"

#define SAFECALL(x) {DWORD internalError=GetLastError();x;SetLastError(internalError);}

BOOL KHOPANEnablePrivilege(const LPCWSTR privilege) {
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

BOOL KHOPANExecuteCommand(const LPCWSTR command, const BOOL block) {
	if(!command) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	LPWSTR fileCommandPrompt = KHOPANFileGetCmd();

	if(!fileCommandPrompt) {
		return FALSE;
	}

	LPWSTR argument = KHOPANFormatMessage(L"%ws /c \"%ws\"", fileCommandPrompt, command);

	if(!argument) {
		SAFECALL(LocalFree(fileCommandPrompt));
		return FALSE;
	}

	BOOL response = KHOPANExecuteProcess(fileCommandPrompt, argument, block);
	SAFECALL(LocalFree(argument));
	SAFECALL(LocalFree(fileCommandPrompt));
	return response;
}

BOOL KHOPANExecuteDynamicLibrary(const LPCWSTR file, const LPCWSTR function, const LPCWSTR argument) {
	if(!file || !function) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	LPWSTR fileRundll32 = KHOPANFileGetRundll32();

	if(!fileRundll32) {
		return FALSE;
	}

	LPWSTR argumentRundll32 = argument ? KHOPANFormatMessage(L"%ws \"%ws\" %ws %ws", fileRundll32, file, function, argument) : KHOPANFormatMessage(L"%ws \"%ws\" %ws", fileRundll32, file, function);

	if(!argumentRundll32) {
		SAFECALL(LocalFree(fileRundll32));
		return FALSE;
	}

	BOOL response = KHOPANExecuteProcess(fileRundll32, argument, FALSE);
	SAFECALL(LocalFree(argument));
	SAFECALL(LocalFree(fileRundll32));
	return response;
}

BOOL KHOPANExecuteProcess(const LPCWSTR file, const LPCWSTR argument, const BOOL block) {
	if(!file) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	LPWSTR argumentMutable = KHOPANStringDuplicate(argument);

	if(!argumentMutable) {
		return FALSE;
	}

	STARTUPINFOW startup = {0};
	startup.cb = sizeof(startup);
	PROCESS_INFORMATION process;
	startup.cb = CreateProcessW(file, argumentMutable, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &startup, &process);
	SAFECALL(LocalFree(argumentMutable));

	if(!startup.cb) {
		return FALSE;
	}

	if(block && WaitForSingleObject(process.hProcess, INFINITE) == WAIT_FAILED) {
		return FALSE;
	}

	if(!CloseHandle(process.hProcess)) {
		return FALSE;
	}

	if(!CloseHandle(process.hThread)) {
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

LPWSTR KHOPANFormatMessage(const LPCWSTR format, ...) {
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

LPWSTR KHOPANStringDuplicate(const LPCWSTR text) {
	if(!text) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return NULL;
	}

	size_t length = wcslen(text);

	if(length < 1) {
		SetLastError(ERROR_INVALID_DATA);
		return NULL;
	}

	LPWSTR buffer = LocalAlloc(LMEM_FIXED, (length + 1) * sizeof(WCHAR));

	if(!buffer) {
		return NULL;
	}

	for(size_t i = 0; i < length; i++) {
		buffer[i] = text[i];
	}

	buffer[length] = 0;
	SetLastError(ERROR_SUCCESS);
	return buffer;
}
