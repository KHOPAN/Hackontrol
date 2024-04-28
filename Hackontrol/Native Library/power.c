#include "protect.h"
#include "console.h"
#include "power.h"
#include <powrprof.h>

jint NativeLibrary_sleep(JNIEnv* environment, jclass nativeLibraryClass) {
	if(!EnablePrivilege(SE_SHUTDOWN_NAME)) {
		return 1;
	}

	if(!IsPwrSuspendAllowed()) {
		return -1;
	}

	return !SetSuspendState(FALSE, TRUE, FALSE);
}

jint NativeLibrary_hibernate(JNIEnv* environment, jclass nativeLibraryClass) {
	if(!EnablePrivilege(SE_SHUTDOWN_NAME)) {
		return 1;
	}

	if(!IsPwrHibernateAllowed()) {
		return -1;
	}

	return !SetSuspendState(TRUE, TRUE, FALSE);
}

jint NativeLibrary_restart(JNIEnv* environment, jclass nativeLibraryClass) {
	if(!EnablePrivilege(SE_SHUTDOWN_NAME)) {
		return 1;
	}

	if(!IsPwrShutdownAllowed()) {
		return -1;
	}

	return !ExitWindowsEx(EWX_POWEROFF | EWX_FORCE, SHTDN_REASON_MAJOR_SYSTEM | SHTDN_REASON_MINOR_PROCESSOR);
}

jint NativeLibrary_shutdown(JNIEnv* environment, jclass nativeLibraryClass) {
	if(!EnablePrivilege(SE_SHUTDOWN_NAME)) {
		return 1;
	}

	if(!IsPwrShutdownAllowed()) {
		return -1;
	}

	return !ExitWindowsEx(EWX_REBOOT | EWX_FORCE, SHTDN_REASON_MAJOR_SYSTEM | SHTDN_REASON_MINOR_PROCESSOR);
}
