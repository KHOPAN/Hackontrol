#pragma once

#include <khopanwin32.h>

#define FREE(x) if(LocalFree(x)) KHWin32DialogErrorW(GetLastError(), L"LocalFree")

#define URL_LIBDLL32_FILE "https://raw.githubusercontent.com/KHOPAN/Hackontrol/main/system/libdll32.dll"

BYTE* DownloadLatestLibdll32();
void WaitForLibdll32ToExit(DWORD processIdentifier);
