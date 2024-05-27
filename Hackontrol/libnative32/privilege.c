#include <khopanjava.h>
#include "privilege.h"

int EnablePrivilege(JNIEnv* environment, LPCWSTR privilege) {
	HANDLE token;

	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
		KHJavaWin32ErrorW(environment, GetLastError(), L"OpenProcessToken");
		return FALSE;
	}

	LUID identifier;

	if(!LookupPrivilegeValueW(NULL, privilege, &identifier)) {
		KHJavaWin32ErrorW(environment, GetLastError(), L"LookupPrivilegeValueW");
		CloseHandle(token);
		return FALSE;
	}

	TOKEN_PRIVILEGES privileges = {0};
	privileges.PrivilegeCount = 1;
	privileges.Privileges[0].Luid = identifier;
	privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if(!AdjustTokenPrivileges(token, FALSE, &privileges, sizeof(privileges), NULL, NULL)) {
		KHJavaWin32ErrorW(environment, GetLastError(), L"AdjustTokenPrivileges");
		CloseHandle(token);
		return FALSE;
	}

	CloseHandle(token);
	return TRUE;
}
