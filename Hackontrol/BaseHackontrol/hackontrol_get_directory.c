#include "hackontrol.h"

#define HACKONTROL_DIRECTORY L"%ProgramData%\\Microsoft\\DeviceSync"

LPWSTR HackontrolGetDirectory() {
	DWORD size = ExpandEnvironmentStringsW(HACKONTROL_DIRECTORY, NULL, 0);

	if(!size) {
		return NULL;
	}

	LPWSTR buffer = LocalAlloc(LMEM_FIXED, size * sizeof(WCHAR));

	if(!buffer) {
		return NULL;
	}

	if(!ExpandEnvironmentStringsW(HACKONTROL_DIRECTORY, buffer, size)) {
		LocalFree(buffer);
		return NULL;
	}

	return buffer;
}
