#include <khopanwin32.h>
#include "Kernel.h"

typedef long (WINAPI* RtlSetProcessIsCritical) (IN BOOLEAN newValue, OUT BOOLEAN* oldValue, IN BOOLEAN criticalBreak);

void Kernel_setProcessCritical(JNIEnv* const environment, const jclass class, const jboolean critical) {
	if(!KHWin32EnablePrivilegeW(SE_DEBUG_NAME)) {
		return;
	}

	HMODULE module = LoadLibraryW(L"ntdll.dll");

	if(!module) {
		return;
	}

	RtlSetProcessIsCritical function = (RtlSetProcessIsCritical) GetProcAddress(module, "RtlSetProcessIsCritical");

	if(!function) {
		return;
	}

	function(critical, NULL, FALSE);
}
