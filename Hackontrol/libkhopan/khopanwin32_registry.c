#include "khopanwin32.h"

BOOL KHWin32RegistrySetStringValueA(const HKEY key, const LPSTR valueName, const LPSTR value) {
	if(!key) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	LSTATUS error = RegSetValueExA(key, valueName, 0, REG_SZ, (const BYTE*) value, value ? (DWORD) ((strlen(value) + 1) * sizeof(CHAR)) : 0);

	if(error) {
		SetLastError(error);
		return FALSE;
	}

	return TRUE;
}

BOOL KHWin32RegistrySetStringValueW(const HKEY key, const LPWSTR valueName, const LPWSTR value) {
	if(!key) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	LSTATUS error = RegSetValueExW(key, valueName, 0, REG_SZ, (const BYTE*) value, value ? (DWORD) ((wcslen(value) + 1) * sizeof(WCHAR)) : 0);

	if(error) {
		SetLastError(error);
		return FALSE;
	}

	return TRUE;
}
