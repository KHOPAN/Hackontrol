#pragma once

#include <Windows.h>

#define FILE_LIBDLL32 L"libdll32.dll"

LPWSTR HackontrolGetHomeDirectory();
BOOL HackontrolWriteFile(const LPCWSTR file, const PBYTE data, const size_t size);
BOOL HackontrolCreateDirectory(const LPCWSTR folder);
