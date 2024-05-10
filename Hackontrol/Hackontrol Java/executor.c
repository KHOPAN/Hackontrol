#include <khopanwin32.h>
#include "executor.h"
#include "uiaccess.h"

void ExecuteJarFile(const LPWSTR javaCommand, const LPWSTR commandArguments, const LPWSTR binaryPath) {
	STARTUPINFO startupInformation = {0};
	startupInformation.cb = sizeof(startupInformation);
	PROCESS_INFORMATION processInformation;
	HANDLE accessToken;

	if(!CreateUIAccessToken(&accessToken)) {
		return;
	}

	if(accessToken) {
		if(!CreateProcessAsUserW(accessToken, javaCommand, commandArguments, NULL, NULL, FALSE, ABOVE_NORMAL_PRIORITY_CLASS, NULL, binaryPath, &startupInformation, &processInformation)) {
			KHWin32DialogErrorW(GetLastError(), L"CreateProcessAsUserW");
			return;
		}
	} else {
		if(!CreateProcessW(javaCommand, commandArguments, NULL, NULL, TRUE, ABOVE_NORMAL_PRIORITY_CLASS, NULL, binaryPath, &startupInformation, &processInformation)) {
			KHWin32DialogErrorW(GetLastError(), L"CreateProcessW");
			return;
		}
	}

	if(WaitForSingleObject(processInformation.hProcess, INFINITE) == WAIT_FAILED) {
		KHWin32DialogErrorW(GetLastError(), L"WaitForSingleObject");
		goto closeHandles;
	}
closeHandles:
	CloseHandle(processInformation.hProcess);
	CloseHandle(processInformation.hThread);
}
