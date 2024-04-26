#include "definition.h"

BOOL EnablePrivilege(LPCWSTR privilege) {
	if(!privilege) {
		return FALSE;
	}

	HANDLE token = NULL;
	LUID identifier;
	TOKEN_PRIVILEGES privileges = {0};

	if(!OpenProcessToken(GetCurrentProcess(), (TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY), &token)) {
		return FALSE;
	}

	if(!LookupPrivilegeValueW(NULL, privilege, &identifier)) {
		return FALSE;
	}

	privileges.PrivilegeCount = 1;
	privileges.Privileges[0].Luid = identifier;
	privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	BOOL success = AdjustTokenPrivileges(token, FALSE, &privileges, sizeof(privileges), NULL, NULL);
	CloseHandle(token);
	return success;
}
