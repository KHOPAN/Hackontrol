#include <khopanwin32.h>
#include "downloadjar.h"

BOOL DownloadJar(const LPWSTR system32Path, const LPWSTR rundll32Path, const LPWSTR downloadArgument) {
	STARTUPINFO startupInformation = {0};
	startupInformation.cb = sizeof(startupInformation);
	PROCESS_INFORMATION processInformation;

	if(!CreateProcessW(rundll32Path, downloadArgument, NULL, NULL, TRUE, ABOVE_NORMAL_PRIORITY_CLASS, NULL, system32Path, &startupInformation, &processInformation)) {
		KHWin32DialogErrorW(GetLastError(), L"CreateProcessW");
		return FALSE;
	}

	BOOL returnValue = FALSE;

	if(WaitForSingleObject(processInformation.hProcess, INFINITE) == WAIT_FAILED) {
		KHWin32DialogErrorW(GetLastError(), L"WaitForSingleObject");
		goto closeHandles;
	}

	returnValue = TRUE;
closeHandles:
	CloseHandle(processInformation.hProcess);
	CloseHandle(processInformation.hThread);
	return returnValue;
}
