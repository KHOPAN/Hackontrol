#include "khopanwin32.h"
#include "khopanstring.h"

BOOL KHWin32StartProcessA(const LPSTR filePath, const LPSTR argument) {
	if(!filePath) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	STARTUPINFOA startupInformation = {0};
	startupInformation.cb = sizeof(startupInformation);
	PROCESS_INFORMATION processInformation;

	if(!CreateProcessA(filePath, argument, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &startupInformation, &processInformation)) {
		return FALSE;
	}

	if(!CloseHandle(processInformation.hProcess)) {
		return FALSE;
	}

	if(!CloseHandle(processInformation.hThread)) {
		return FALSE;
	}

	return TRUE;
}

BOOL KHWin32StartProcessW(const LPWSTR filePath, const LPWSTR argument) {
	if(!filePath) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	STARTUPINFOW startupInformation = {0};
	startupInformation.cb = sizeof(startupInformation);
	PROCESS_INFORMATION processInformation;

	if(!CreateProcessW(filePath, argument, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &startupInformation, &processInformation)) {
		return FALSE;
	}

	if(!CloseHandle(processInformation.hProcess)) {
		return FALSE;
	}

	if(!CloseHandle(processInformation.hThread)) {
		return FALSE;
	}

	return TRUE;
}

LPSTR KHWin32GetRundll32FileA() {
	LPSTR pathFolderWindows = KHWin32GetWindowsDirectoryA();

	if(!pathFolderWindows) {
		return NULL;
	}

	LPSTR pathFileRundll32 = KHFormatMessageA("%s\\%ws\\%ws", pathFolderWindows, FOLDER_SYSTEM32, FILE_RUNDLL32);
	LocalFree(pathFolderWindows);
	return pathFileRundll32;
}

LPWSTR KHWin32GetRundll32FileW() {
	LPWSTR pathFolderWindows = KHWin32GetWindowsDirectoryW();

	if(!pathFolderWindows) {
		return NULL;
	}

	LPWSTR pathFileRundll32 = KHFormatMessageW(L"%ws\\" FOLDER_SYSTEM32 L"\\" FILE_RUNDLL32, pathFolderWindows);
	LocalFree(pathFolderWindows);
	return pathFileRundll32;
}

BOOL KHWin32StartDynamicLibraryA(const LPSTR filePath, const LPSTR functionName, const LPSTR argument) {
	if(!filePath || !functionName) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	LPSTR pathFileRundll32 = KHWin32GetRundll32FileA();

	if(!pathFileRundll32) {
		return FALSE;
	}

	LPSTR argumentFileRundll32;

	if(argument) {
		argumentFileRundll32 = KHFormatMessageA("%s \"%s\" %s %s", pathFileRundll32, filePath, functionName, argument);
	} else {
		argumentFileRundll32 = KHFormatMessageA("%s \"%s\" %s", pathFileRundll32, filePath, functionName);
	}

	if(!argumentFileRundll32) {
		LocalFree(pathFileRundll32);
		return FALSE;
	}

	BOOL result = KHWin32StartProcessA(pathFileRundll32, argumentFileRundll32);
	LocalFree(argumentFileRundll32);
	LocalFree(pathFileRundll32);
	return result;
}

BOOL KHWin32StartDynamicLibraryW(const LPWSTR filePath, const LPWSTR functionName, const LPWSTR argument) {
	if(!filePath || !functionName) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	LPWSTR pathFileRundll32 = KHWin32GetRundll32FileW();

	if(!pathFileRundll32) {
		return FALSE;
	}

	LPWSTR argumentFileRundll32;

	if(argument) {
		argumentFileRundll32 = KHFormatMessageW(L"%ws \"%ws\" %ws %ws", pathFileRundll32, filePath, functionName, argument);
	} else {
		argumentFileRundll32 = KHFormatMessageW(L"%ws \"%ws\" %ws", pathFileRundll32, filePath, functionName);
	}

	if(!argumentFileRundll32) {
		LocalFree(pathFileRundll32);
		return FALSE;
	}

	BOOL result = KHWin32StartProcessW(pathFileRundll32, argumentFileRundll32);
	LocalFree(argumentFileRundll32);
	LocalFree(pathFileRundll32);
	return result;
}

LPSTR KHWin32GetCmdFileA() {
	LPSTR pathFolderWindows = KHWin32GetWindowsDirectoryA();

	if(!pathFolderWindows) {
		return NULL;
	}

	LPSTR pathFileCmd = KHFormatMessageA("%s\\%ws\\%ws", pathFolderWindows, FOLDER_SYSTEM32, FILE_CMD);
	LocalFree(pathFolderWindows);
	return pathFileCmd;
}

LPWSTR KHWin32GetCmdFileW() {
	LPWSTR pathFolderWindows = KHWin32GetWindowsDirectoryW();

	if(!pathFolderWindows) {
		return NULL;
	}

	LPWSTR pathFileCmd = KHFormatMessageW(L"%ws\\" FOLDER_SYSTEM32 L"\\" FILE_CMD, pathFolderWindows);
	LocalFree(pathFolderWindows);
	return pathFileCmd;
}

BOOL KHWin32ExecuteCommandA(const LPSTR command) {
	if(!command) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	LPSTR pathFileCmd = KHWin32GetCmdFileA();

	if(!pathFileCmd) {
		return FALSE;
	}

	LPSTR argumentFileCmd = KHFormatMessageA("%s /c \"%s\"", pathFileCmd, command);

	if(!argumentFileCmd) {
		LocalFree(pathFileCmd);
		return FALSE;
	}

	BOOL result = KHWin32StartProcessA(pathFileCmd, argumentFileCmd);
	LocalFree(argumentFileCmd);
	LocalFree(pathFileCmd);
	return result;
}

BOOL KHWin32ExecuteCommandW(const LPWSTR command) {
	if(!command) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	LPWSTR pathFileCmd = KHWin32GetCmdFileW();

	if(!pathFileCmd) {
		return FALSE;
	}

	LPWSTR argumentFileCmd = KHFormatMessageW(L"%ws /c \"%ws\"", pathFileCmd, command);

	if(!argumentFileCmd) {
		LocalFree(pathFileCmd);
		return FALSE;
	}

	BOOL result = KHWin32StartProcessW(pathFileCmd, argumentFileCmd);
	LocalFree(argumentFileCmd);
	LocalFree(pathFileCmd);
	return result;
}
