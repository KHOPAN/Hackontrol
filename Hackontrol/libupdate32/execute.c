#include "update.h"
#include <khopanstring.h>

void ExecuteLibdll32() {
	LPWSTR pathFolderWindows = KHWin32GetWindowsDirectoryW();

	if(!pathFolderWindows) {
		KHWin32DialogErrorW(GetLastError(), L"KHWin32GetWindowsDirectoryW");
		return;
	}

	LPWSTR pathFolderSystem32 = KHFormatMessageW(L"%ws\\" FOLDER_SYSTEM32, pathFolderWindows);
	FREE(pathFolderWindows);

	if(!pathFolderSystem32) {
		KHWin32DialogErrorW(GetLastError(), L"KHFormatMessageW");
		return;
	}

	LPWSTR pathFileRundll32 = KHFormatMessageW(L"%ws\\" FILE_RUNDLL32, pathFolderSystem32);

	if(!pathFileRundll32) {
		KHWin32DialogErrorW(GetLastError(), L"KHFormatMessageW");
		FREE(pathFolderSystem32);
		return;
	}

	STARTUPINFO startupInformation = {0};
	startupInformation.cb = sizeof(startupInformation);
	PROCESS_INFORMATION processInformation;
	BOOL result = CreateProcessW(pathFileRundll32, FILE_RUNDLL32 L" " FILE_LIBDLL32 L"," FUNCTION_LIBDLL32, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, pathFolderSystem32, &startupInformation, &processInformation);
	FREE(pathFileRundll32);
	FREE(pathFolderSystem32);

	if(!result) {
		KHWin32DialogErrorW(GetLastError(), L"CreateProcessW");
		return;
	}

	if(!CloseHandle(processInformation.hProcess)) {
		KHWin32DialogErrorW(GetLastError(), L"CloseHandle");
		return;
	}

	if(!CloseHandle(processInformation.hThread)) {
		KHWin32DialogErrorW(GetLastError(), L"CloseHandle");
	}
}
