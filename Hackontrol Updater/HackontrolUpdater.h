#pragma once

#define CURL_STATICLIB
#include <curl/curl.h>

#ifdef __cplusplus
extern "C" {
#endif
void HU_DisplayError(DWORD, const wchar_t*);
void HU_CURLError(CURLcode, const char*);
CURL* HU_InitializeCURL();
const char* HU_GetVersionFile(CURL*);
BOOL HU_IsFileExists(const wchar_t*);
const char* HU_Hash(BYTE*, size_t);
wchar_t* HU_GetSystemDirectory(const wchar_t*);
void HU_DownloadFile(CURL*, const char*, const wchar_t*);
#ifdef __cplusplus
}
#endif
