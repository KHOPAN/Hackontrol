#include "installer.h"

wchar_t* mergePath(const wchar_t* directory, const wchar_t* fileName) {
	size_t directoryLength = wcslen(directory);
	size_t fileNameLength = wcslen(fileName);
	int notEndWithBackslash = directory[directoryLength] != L'\\';
	size_t bufferSize = directoryLength + fileNameLength + notEndWithBackslash + 1;
	wchar_t* buffer = malloc(bufferSize * sizeof(wchar_t));
	
	if(!buffer) {
		consoleError(ERROR_OUTOFMEMORY, L"malloc");
		ExitProcess(1);
		return NULL;
	}
	
	for(size_t i = 0; i < directoryLength; i++) {
		buffer[i] = directory[i];
	}

	if(notEndWithBackslash) {
		buffer[directoryLength] = L'\\';
	}

	for(size_t i = 0; i < fileNameLength; i++) {
		buffer[i + directoryLength + notEndWithBackslash] = fileName[i];
	}

	buffer[bufferSize - 1] = 0;
	return buffer;
}
