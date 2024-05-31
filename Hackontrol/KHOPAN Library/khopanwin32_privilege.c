#include "khopanwin32.h"

static BOOL adjustPrivilege(const HANDLE token, const LUID identifier) {
	TOKEN_PRIVILEGES privileges = {0};
	privileges.PrivilegeCount = 1;
	privileges.Privileges[0].Luid = identifier;
	privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if(!AdjustTokenPrivileges(token, FALSE, &privileges, sizeof(privileges), NULL, NULL)) {
		CloseHandle(token);
		return FALSE;
	}

	CloseHandle(token);
	return TRUE;
}

BOOL KHWin32EnablePrivilegeA(const LPSTR privilege) {
	if(!privilege) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	HANDLE token;

	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
		return FALSE;
	}

	LUID identifier;

	if(!LookupPrivilegeValueA(NULL, privilege, &identifier)) {
		CloseHandle(token);
		return FALSE;
	}

	return adjustPrivilege(token, identifier);
}

BOOL KHWin32EnablePrivilegeW(const LPWSTR privilege) {
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
		CloseHandle(token);
		return FALSE;
	}

	return adjustPrivilege(token, identifier);
}
