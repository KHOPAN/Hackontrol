#include "execute.h"
#include <khopanstring.h>

void ProcessEntrypointShell(cJSON* root) {
	if(!cJSON_HasObjectItem(root, "command")) {
		return;
	}

	cJSON* command = cJSON_GetObjectItem(root, "command");

	if(!cJSON_IsString(command)) {
		return;
	}

	LPWSTR pathFolderWindows = KHWin32GetWindowsDirectoryW();

	if(!pathFolderWindows) {
		return;
	}

	LPWSTR pathFileCmd = KHFormatMessageW(L"%ws\\" FOLDER_SYSTEM32 L"\\" FILE_CMD, pathFolderWindows);
	LocalFree(pathFolderWindows);

	if(!pathFileCmd) {
		return;
	}

	LPWSTR argumentFileCmd = KHFormatMessageW(FILE_CMD L" /c %S", cJSON_GetStringValue(command));

	if(!argumentFileCmd) {
		LocalFree(pathFileCmd);
		return;
	}

	StartProcess(pathFileCmd, argumentFileCmd);
	LocalFree(argumentFileCmd);
	LocalFree(pathFileCmd);
}
