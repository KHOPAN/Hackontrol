#include <AccCtrl.h>
#include <AclAPI.h>
#include "protect.h"

typedef long (WINAPI* RtlSetProcessIsCritical) (IN BOOLEAN newValue, OUT BOOLEAN* oldValue, IN BOOLEAN criticalBreak);

void ProtectProcess() {
	if(!EnablePrivilege(SE_DEBUG_NAME)) {
		return;
	}

	HMODULE handle = LoadLibraryW(L"ntdll.dll");

	if(!handle) {
		return;
	}

	RtlSetProcessIsCritical SetProcessIsCritical = (RtlSetProcessIsCritical) GetProcAddress(handle, "RtlSetProcessIsCritical");

	if(!SetProcessIsCritical) {
		return;
	}

	SetProcessIsCritical(TRUE, NULL, FALSE);
	HANDLE process = GetCurrentProcess();
	PACL list = LocalAlloc(LMEM_FIXED, sizeof(ACL));

	if(!list) {
		return;
	}

	InitializeAcl(list, sizeof(ACL), ACL_REVISION);
	SetSecurityInfo(process, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, list, NULL);
	LocalFree(list);
}

int EnablePrivilege(LPCWSTR privilege) {
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
