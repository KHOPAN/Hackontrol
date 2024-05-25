#pragma once

#include <khopandatastream.h>

#define FILE_LIBDLL32 L"libdll32.dll"

LPWSTR HackontrolGetDirectory();
BOOL HackontrolWriteFile(LPCWSTR const filePath, const DataStream* const stream);
