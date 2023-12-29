#pragma once

#include <Windows.h>

#define CURL_STATICLIB
#include <curl/curl.h>

#define EXPORT __declspec(dllexport) void __stdcall

#ifdef __cplusplus
extern "C" {
#endif
EXPORT Execute(HWND, HINSTANCE, LPSTR, int);
void HU_CURLError(CURLcode, const char*);
void HU_DisplayError(DWORD, const wchar_t*);
void HU_DownloadFile(CURL*, const char*, const wchar_t*);
wchar_t* HU_GetSystemDirectory(const wchar_t*);
const char* HU_GetVersionFile(CURL*);
const char* HU_Hash(BYTE*, size_t);
CURL* HU_InitializeCURL();
BOOL HU_IsFileExists(const wchar_t*);
#ifdef __cplusplus
}
#endif
