#include <string>
#include "HackontrolUpdater.h"

const wchar_t* HU_GetSystemDirectory(const wchar_t* fileName) {
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
		ExitProcess(-1);
		return NULL;
	}

	std::wstring string(buffer);
	free(buffer);

	if(!string.empty() && string.at(string.length() - 1) != L'\\') {
		string += '\\';
	}

	string += L"System32\\";
	string += fileName;
	const wchar_t* filePath = string.c_str();
	return filePath;
}
