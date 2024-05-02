#include <khopanerror.h>
#include "jardownloader.h"

BOOL DownloadJar(LPWSTR system32Path, LPWSTR rundll32Path, LPWSTR downloadArgument) {
	STARTUPINFO startup = {0};
	startup.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION information;

	if(!CreateProcessW(rundll32Path, downloadArgument, NULL, NULL, TRUE, ABOVE_NORMAL_PRIORITY_CLASS, NULL, system32Path, &startup, &information)) {
		KHWin32DialogErrorW(GetLastError(), L"CreateProcessW");
		return TRUE;
	}

	if(WaitForSingleObject(information.hProcess, INFINITE) == WAIT_FAILED) {
		KHWin32DialogErrorW(GetLastError(), L"WaitForSingleObject");
		return TRUE;
	}

	if(!CloseHandle(information.hProcess)) {
		KHWin32DialogErrorW(GetLastError(), L"CloseHandle");
		return TRUE;
	}

	if(!CloseHandle(information.hThread)) {
		KHWin32DialogErrorW(GetLastError(), L"CloseHandle");
		return TRUE;
	}

	return FALSE;
}
