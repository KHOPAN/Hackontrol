#include "installer.h"

wchar_t* getSystem32Directory() {
	UINT size = GetWindowsDirectoryW(NULL, 0);

	if(!size) {
		consoleError(GetLastError(), L"GetWindowsDirectoryW");
		ExitProcess(1);
		return NULL;
	}

	wchar_t* buffer = malloc(size * sizeof(wchar_t));

	if(!buffer) {
		consoleError(ERROR_OUTOFMEMORY, L"malloc");
		ExitProcess(1);
		return NULL;
	}

	size = GetWindowsDirectoryW(buffer, size);

	if(!size) {
		consoleError(GetLastError(), L"GetWindowsDirectoryW");
		ExitProcess(1);
		return NULL;
	}

	wchar_t* result = mergePath(buffer, L"System32");
	free(buffer);
	return result;
}
