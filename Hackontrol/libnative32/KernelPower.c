#include <khopanwin32.h>
#include <powrprof.h>
#include "Kernel.h"

#pragma warning(disable: 28159)

#define PRIVILEGE if(!enablePrivilege())return;

static BOOL enablePrivilege() {
	if(!KHWin32EnablePrivilegeW(SE_SHUTDOWN_NAME)) {
		return FALSE;
	}

	return TRUE;
}

static void sleepHackontrol(BOOL hibernate) {
	PRIVILEGE;
	SetSuspendState(hibernate, TRUE, FALSE);
}

static void shutdownHackontrol(BOOL restart) {
	PRIVILEGE;
	UINT flags = EWX_FORCE;

	if(restart) {
		flags |= EWX_REBOOT;
	} else {
		flags |= EWX_SHUTDOWN;
	}

	ExitWindowsEx(flags, SHTDN_REASON_MAJOR_SYSTEM | SHTDN_REASON_MINOR_PROCESSOR);
}

void Kernel_sleep(JNIEnv* const environment, const jclass class) {
	sleepHackontrol(FALSE);
}

void Kernel_hibernate(JNIEnv* const environment, const jclass class) {
	sleepHackontrol(TRUE);
}

void Kernel_shutdown(JNIEnv* const environment, const jclass class) {
	shutdownHackontrol(FALSE);
}

void Kernel_restart(JNIEnv* const environment, const jclass class) {
	shutdownHackontrol(TRUE);
}
