#include <string>
#include "HackontrolUpdater.h"

const wchar_t* HU_GetSystemDirectory(const wchar_t* fileName) {
	INT size = GetWindowsDirectoryW(NULL, 0);
	LPWSTR buffer = static_cast<LPWSTR>(malloc(sizeof(wchar_t) * size));
	GetWindowsDirectoryW(buffer, size);
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
