#include <AccCtrl.h>
#include <AclAPI.h>
#include "native.h"
#include "privilege.h"

typedef long (WINAPI* RtlSetProcessIsCritical) (IN BOOLEAN newValue, OUT BOOLEAN* oldValue, IN BOOLEAN criticalBreak);

jboolean NativeLibrary_critical(JNIEnv* environment, jclass nativeLibraryClass, jboolean critical) {
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

	BOOLEAN oldValue = FALSE;
	SetProcessIsCritical(critical, &oldValue, FALSE);
	HANDLE process = GetCurrentProcess();
	PACL list = LocalAlloc(LMEM_FIXED, sizeof(ACL));

	if(!list) {
		return oldValue;
	}

	InitializeAcl(list, sizeof(ACL), ACL_REVISION);
	SetSecurityInfo(process, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, list, NULL);
	LocalFree(list);
	return oldValue;
}
