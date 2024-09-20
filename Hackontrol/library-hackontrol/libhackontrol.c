#include "libhackontrol.h"
#include <ShlObj_core.h>

#define HACKONTROL_DIRECTORY L"%LOCALAPPDATA%\\Microsoft\\InstallService"

LPWSTR HackontrolGetHomeDirectory() {
	DWORD size = ExpandEnvironmentStringsW(HACKONTROL_DIRECTORY, NULL, 0);

	if(!size) {
		return NULL;
	}

	LPWSTR buffer = LocalAlloc(LMEM_FIXED, size * sizeof(WCHAR));

	if(!buffer) {
		return NULL;
	}

	if(!ExpandEnvironmentStringsW(HACKONTROL_DIRECTORY, buffer, size)) {
		DWORD error = GetLastError();
		LocalFree(buffer);
		SetLastError(error);
		return NULL;
	}

	SetLastError(ERROR_SUCCESS);
	return buffer;
}

BOOL HackontrolWriteFile(const LPCWSTR file, const PBYTE data, const size_t size) {
	if(!file || !data || !size) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	HANDLE handleFile = CreateFileW(file, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(handleFile == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	DWORD written;
	BOOL returnValue = FALSE;

	if(!WriteFile(handleFile, data, (DWORD) size, &written, NULL)) {
		goto closeFile;
	}

	if(size != written) {
		SetLastError(ERROR_FUNCTION_FAILED);
		goto closeFile;
	}

	SetLastError(ERROR_SUCCESS);
	returnValue = TRUE;
closeFile:
	CloseHandle(handleFile);
	return returnValue;
}

BOOL HackontrolCreateDirectory(const LPCWSTR folder) {
	if(!folder) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	int code = SHCreateDirectoryExW(NULL, folder, NULL);

	if(code && code != ERROR_FILE_EXISTS && code != ERROR_ALREADY_EXISTS) {
		SetLastError(code);
		return FALSE;
	}

	SetLastError(ERROR_SUCCESS);
	return TRUE;
}
