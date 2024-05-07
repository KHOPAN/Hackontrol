#include "khopanwin32.h"

LPSTR KHWin32GetWindowsDirectoryA() {
	UINT size = GetSystemWindowsDirectoryA(NULL, 0);

	if(!size) {
		return NULL;
	}

	LPSTR buffer = LocalAlloc(LMEM_FIXED, size);

	if(!buffer) {
		return NULL;
	}

	if(!GetSystemWindowsDirectoryA(buffer, size)) {
		return NULL;
	}

	return buffer;
}

LPWSTR KHWin32GetWindowsDirectoryW() {
	UINT size = GetSystemWindowsDirectoryW(NULL, 0);

	if(!size) {
		return NULL;
	}

	LPWSTR buffer = LocalAlloc(LMEM_FIXED, size);

	if(!buffer) {
		return NULL;
	}

	if(!GetSystemWindowsDirectoryW(buffer, size)) {
		return NULL;
	}

	return buffer;
}
