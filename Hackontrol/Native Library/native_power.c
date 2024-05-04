#include "privilege.h"
#include "native_power.h"
#include <powrprof.h>
#include <khopanerror.h>

jstring NativeLibrary_sleep(JNIEnv* environment, jclass nativeLibraryClass) {
	if(!EnablePrivilege(SE_SHUTDOWN_NAME)) {
		return (*environment)->NewString(environment, L"Error while enabling shutdown privilege", 39);
	}

	if(!IsPwrSuspendAllowed()) {
		return (*environment)->NewString(environment, L"Sleep is not supported on this system", 37);
	}

	if(SetSuspendState(FALSE, TRUE, FALSE)) {
		return NULL;
	}

	LPWSTR message = KHGetWin32ErrorMessageW(GetLastError(), L"SetSuspendState");
	jstring string = (*environment)->NewString(environment, message, (jsize) wcslen(message));
	LocalFree(message);
	return string;
}

jstring NativeLibrary_hibernate(JNIEnv* environment, jclass nativeLibraryClass) {
	if(!EnablePrivilege(SE_SHUTDOWN_NAME)) {
		return (*environment)->NewString(environment, L"Error while enabling shutdown privilege", 39);
	}

	if(!IsPwrHibernateAllowed()) {
		return (*environment)->NewString(environment, L"Hibernate is not supported on this system", 37);
	}

	if(SetSuspendState(TRUE, TRUE, FALSE)) {
		return NULL;
	}

	LPWSTR message = KHGetWin32ErrorMessageW(GetLastError(), L"SetSuspendState");
	jstring string = (*environment)->NewString(environment, message, (jsize) wcslen(message));
	LocalFree(message);
	return string;
}

jstring NativeLibrary_restart(JNIEnv* environment, jclass nativeLibraryClass) {
	if(!EnablePrivilege(SE_SHUTDOWN_NAME)) {
		return (*environment)->NewString(environment, L"Error while enabling shutdown privilege", 39);
	}

	if(!IsPwrShutdownAllowed()) {
		return (*environment)->NewString(environment, L"Shutdown is not supported on this system (weird)", 48);
	}

	if(ExitWindowsEx(EWX_POWEROFF | EWX_FORCE, SHTDN_REASON_MAJOR_SYSTEM | SHTDN_REASON_MINOR_PROCESSOR)) {
		return NULL;
	}

	LPWSTR message = KHGetWin32ErrorMessageW(GetLastError(), L"ExitWindowsEx");
	jstring string = (*environment)->NewString(environment, message, (jsize) wcslen(message));
	LocalFree(message);
	return string;
}

jstring NativeLibrary_shutdown(JNIEnv* environment, jclass nativeLibraryClass) {
	if(!EnablePrivilege(SE_SHUTDOWN_NAME)) {
		return (*environment)->NewString(environment, L"Error while enabling shutdown privilege", 39);
	}

	if(!IsPwrShutdownAllowed()) {
		return (*environment)->NewString(environment, L"Shutdown is not supported on this system (weird)", 48);
	}

	if(ExitWindowsEx(EWX_REBOOT | EWX_FORCE, SHTDN_REASON_MAJOR_SYSTEM | SHTDN_REASON_MINOR_PROCESSOR)) {
		return NULL;
	}

	LPWSTR message = KHGetWin32ErrorMessageW(GetLastError(), L"ExitWindowsEx");
	jstring string = (*environment)->NewString(environment, message, (jsize) wcslen(message));
	LocalFree(message);
	return string;
}
