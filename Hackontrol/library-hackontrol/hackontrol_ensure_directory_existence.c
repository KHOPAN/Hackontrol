#include "hackontrol.h"
#include <ShlObj_core.h>

BOOL HackontrolEnsureDirectoryExistence(LPCWSTR const folderPath) {
	if(!folderPath) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	int code = SHCreateDirectoryExW(NULL, folderPath, NULL);

	if(code && code != ERROR_FILE_EXISTS && code != ERROR_ALREADY_EXISTS) {
		SetLastError(code);
		return FALSE;
	}

	return TRUE;
}
