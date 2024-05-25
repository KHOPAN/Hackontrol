#pragma once

#include <khopandatastream.h>
#include <khopancurl.h>

#define FILE_LIBDLL32 L"libdll32.dll"

LPWSTR GetHackontrolDirectory();
BOOL HackontrolDownloadData(DataStream* const stream, LPCSTR url, BOOL curlInitialized, CURLcode* const outputCode);
