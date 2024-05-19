#include "execute.h"
#include <khopanstring.h>

BOOL CheckAndProcessSelfUpdate(cJSON* root) {
	if(!cJSON_HasObjectItem(root, "self")) {
		return TRUE;
	}

	cJSON* selfObject = cJSON_GetObjectItem(root, "self");

	if(!cJSON_HasObjectItem(selfObject, "url")) {
		return TRUE;
	}

	LPWSTR pathFolderWindows = KHWin32GetWindowsDirectoryW();

	if(!pathFolderWindows) {
		return TRUE;
	}

	LPWSTR pathFileLibdll32 = KHFormatMessageW(L"%ws\\" FOLDER_SYSTEM32 L"\\" FILE_LIBDLL32, pathFolderWindows);
	LocalFree(pathFolderWindows);

	if(!pathFileLibdll32) {
		return TRUE;
	}

	BOOL result = CheckFileHash(selfObject, pathFileLibdll32);
	LocalFree(pathFileLibdll32);

	if(result) {
		return TRUE;
	}

	MessageBoxW(NULL, L"Hash not match", L"libdll32", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	return TRUE;
}
