#include <Windows.h>
#include "definition.h"

BOOL HU_IsFileExists(const wchar_t* fileName) {
	DWORD attributes = GetFileAttributesW(fileName);
	return attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}
