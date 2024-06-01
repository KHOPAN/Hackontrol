#include <khopanwin32.h>
#include <powrprof.h>
#include "exception.h"
#include "Kernel.h"

#pragma warning(disable: 28159)

static void sleepHackontrol(JNIEnv* const environment, BOOL hibernate) {
	if(!KHWin32EnablePrivilegeW(SE_SHUTDOWN_NAME)) {
		HackontrolThrowWin32Error(environment, L"KHWin32EnablePrivilegeW");
		return;
	}

	if(!SetSuspendState(hibernate, TRUE, FALSE)) {
		HackontrolThrowWin32Error(environment, L"SetSuspendState");
	}
}

static void shutdownHackontrol(JNIEnv* const environment, BOOL restart) {
	if(!KHWin32EnablePrivilegeW(SE_SHUTDOWN_NAME)) {
		HackontrolThrowWin32Error(environment, L"KHWin32EnablePrivilegeW");
		return;
	}

	UINT flags = EWX_FORCE;

	if(restart) {
		flags |= EWX_REBOOT;
	} else {
		flags |= EWX_SHUTDOWN;
	}

	if(!ExitWindowsEx(flags, SHTDN_REASON_MAJOR_SYSTEM | SHTDN_REASON_MINOR_PROCESSOR)) {
		HackontrolThrowWin32Error(environment, L"ExitWindowsEx");
	}
}

void Kernel_sleep(JNIEnv* const environment, const jclass class) {
	sleepHackontrol(environment, FALSE);
}

void Kernel_hibernate(JNIEnv* const environment, const jclass class) {
	sleepHackontrol(environment, TRUE);
}

void Kernel_restart(JNIEnv* const environment, const jclass class) {
	shutdownHackontrol(environment, TRUE);
}

void Kernel_shutdown(JNIEnv* const environment, const jclass class) {
	shutdownHackontrol(environment, FALSE);
}
