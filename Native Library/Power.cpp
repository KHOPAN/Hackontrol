#include <Windows.h>
#include <jni.h>
#include "com_khopan_hackontrol_nativelib_power_Power.h"

JNIEXPORT jint JNICALL Java_com_khopan_hackontrol_nativelib_power_Power_ExitWindowsEx(JNIEnv* environment, jclass powerManagementClass, jint uFlags, jlong dwReason) {
	HANDLE processHandle = GetCurrentProcess();
	HANDLE token;
	TOKEN_PRIVILEGES privileges;
	OpenProcessToken(processHandle, TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &token);
	LookupPrivilegeValueW(NULL, SE_SHUTDOWN_NAME, &privileges.Privileges[0].Luid);
	privileges.PrivilegeCount = 1;
	privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges(token, FALSE, &privileges, 0, NULL, 0);
	return ExitWindowsEx(uFlags, dwReason);
}
