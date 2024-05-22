#include "khopanwin32.h"

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
