#include <khopanerror.h>
#include "executor.h"

void ExecuteJarFile(const LPWSTR javaCommand, const LPWSTR commandArguments) {
	STARTUPINFO startup = {0};
	startup.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION information;

	if(!CreateProcessW(javaCommand, commandArguments, NULL, NULL, TRUE, ABOVE_NORMAL_PRIORITY_CLASS, NULL, NULL, &startup, &information)) {
		KHWin32DialogErrorW(GetLastError(), L"CreateProcessW");
		return;
	}

	if(WaitForSingleObject(information.hProcess, INFINITE) == WAIT_FAILED) {
		KHWin32DialogErrorW(GetLastError(), L"WaitForSingleObject");
		return;
	}

	if(!CloseHandle(information.hProcess)) {
		KHWin32DialogErrorW(GetLastError(), L"CloseHandle");
		return;
	}

	if(!CloseHandle(information.hThread)) {
		KHWin32DialogErrorW(GetLastError(), L"CloseHandle");
	}
}
