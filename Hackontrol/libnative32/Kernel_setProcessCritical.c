#include <khopanwin32.h>
#include "exception.h"
#include "Kernel.h"

typedef long (WINAPI* RtlSetProcessIsCritical) (IN BOOLEAN newValue, OUT BOOLEAN* oldValue, IN BOOLEAN criticalBreak);

void Kernel_setProcessCritical(JNIEnv* const environment, const jclass class, const jboolean critical) {
	if(!KHWin32EnablePrivilegeW(SE_DEBUG_NAME)) {
		HackontrolThrowWin32Error(environment, L"KHWin32EnablePrivilegeW");
		return;
	}

	HMODULE module = LoadLibraryW(L"ntdll.dll");

	if(!module) {
		HackontrolThrowWin32Error(environment, L"LoadLibraryW");
		return;
	}

	RtlSetProcessIsCritical function = (RtlSetProcessIsCritical) GetProcAddress(module, "RtlSetProcessIsCritical");

	if(!function) {
		HackontrolThrowWin32Error(environment, L"GetProcAddress");
		return;
	}

	function(critical, NULL, FALSE);
}
