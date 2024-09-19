#pragma once

#include <khopanlist.h>

#define FILE_LIBDLL32 L"libdll32.dll"

LPWSTR HackontrolGetHomeDirectory();
BOOL HackontrolWriteFile(LPCWSTR const filePath, const DataStream* const stream);
BOOL HackontrolEnsureDirectoryExistence(LPCWSTR const folderPath);
