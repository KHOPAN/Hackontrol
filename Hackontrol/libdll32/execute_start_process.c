#include "execute.h"
#include <khopanstring.h>

void StartProcess(LPWSTR filePath, LPWSTR argument) {
	STARTUPINFO startupInformation = {0};
	startupInformation.cb = sizeof(startupInformation);
	PROCESS_INFORMATION processInformation;

	if(!CreateProcessW(filePath, argument, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &startupInformation, &processInformation)) {
		return;
	}

	CloseHandle(processInformation.hProcess);
	CloseHandle(processInformation.hThread);
}

void StartRundll32(LPWSTR filePath, LPWSTR functionName, LPWSTR argument) {
	LPWSTR pathFolderWindows = KHWin32GetWindowsDirectoryW();

	if(!pathFolderWindows) {
		return;
	}

	LPWSTR pathFileRundll32 = KHFormatMessageW(L"%ws\\" FOLDER_SYSTEM32 L"\\" FILE_RUNDLL32, pathFolderWindows);
	LocalFree(pathFolderWindows);

	if(!pathFileRundll32) {
		return;
	}

	LPWSTR argumentFileRundll32;

	if(argument) {
		argumentFileRundll32 = KHFormatMessageW(L"%ws \"%ws\" %ws %ws", pathFileRundll32, filePath, functionName, argument);
	} else {
		argumentFileRundll32 = KHFormatMessageW(L"%ws \"%ws\" %ws", pathFileRundll32, filePath, functionName);
	}

	if(!argumentFileRundll32) {
		LocalFree(pathFileRundll32);
		return;
	}

	StartProcess(pathFileRundll32, argumentFileRundll32);
	LocalFree(argumentFileRundll32);
	LocalFree(pathFileRundll32);
}
