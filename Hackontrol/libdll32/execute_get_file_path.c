#include "execute.h"
#include <khopanstring.h>

static BOOL isAbsolute(cJSON* root);

LPWSTR GetFilePath(cJSON* root) {
	if(!cJSON_HasObjectItem(root, "file")) {
		return NULL;
	}

	cJSON* file = cJSON_GetObjectItem(root, "file");

	if(!cJSON_IsString(file)) {
		return NULL;
	}

	char* fileValue = cJSON_GetStringValue(file);

	if(isAbsolute(root)) {
		return KHFormatMessageW(L"%S", fileValue);
	}

	LPWSTR pathFolderWindows = KHWin32GetWindowsDirectoryW();

	if(!pathFolderWindows) {
		return NULL;
	}

	LPWSTR filePath = KHFormatMessageW(L"%ws\\" FOLDER_SYSTEM32 L"\\%S", pathFolderWindows, fileValue);
	LocalFree(pathFolderWindows);
	return filePath;
}

static BOOL isAbsolute(cJSON* root) {
	if(cJSON_HasObjectItem(root, "absolute")) {
		cJSON* item = cJSON_GetObjectItem(root, "absolute");

		if(cJSON_IsBool(item)) {
			return cJSON_IsTrue(item);
		}
	}

	if(cJSON_HasObjectItem(root, "relative")) {
		cJSON* item = cJSON_GetObjectItem(root, "relative");

		if(cJSON_IsBool(item)) {
			return cJSON_IsFalse(item);
		}
	}

	return FALSE;
}
