#include <string>
#include "HackontrolUpdater.h"

wchar_t* HU_GetSystemDirectory(const wchar_t* fileName) {
	INT size = GetWindowsDirectoryW(NULL, 0);

	if(size == NULL) {
		HU_DisplayError(GetLastError(), L"GetWindowsDirectoryW()");
		ExitProcess(-1);
		return NULL;
	}

	LPWSTR buffer = static_cast<LPWSTR>(malloc(sizeof(wchar_t) * size));

	if(buffer == NULL) {
		HU_DisplayError(ERROR_NOT_ENOUGH_MEMORY, L"malloc()");
		ExitProcess(-2);
		return NULL;
	}

	size = GetWindowsDirectoryW(buffer, size);

	if(size == NULL) {
		HU_DisplayError(GetLastError(), L"GetWindowsDirectoryW()");
		ExitProcess(-3);
		return NULL;
	}

	std::wstring string(buffer);
	free(buffer);

	if(!string.empty() && string.at(string.length() - 1) != L'\\') {
		string += '\\';
	}

	string += L"System32\\";
	string += fileName;
	size_t length = string.length();
	wchar_t* filePath = static_cast<wchar_t*>(malloc(sizeof(wchar_t) * (length + 1)));
	
	if(filePath == NULL) {
		HU_DisplayError(ERROR_NOT_ENOUGH_MEMORY, L"malloc()");
		ExitProcess(-15);
		return NULL;
	}

	for(size_t i = 0; i < length; i++) {
		filePath[i] = string.at(i);
	}

	filePath[length] = '\u0000';
	return filePath;
}
