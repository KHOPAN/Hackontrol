#pragma once

#include <Windows.h>

#define CURL_STATICLIB
#include <curl/curl.h>

#ifdef __cplusplus
extern "C" {
#endif
LPSTR KHCURLGetErrorMessageA(DWORD errorCode, const LPSTR functionName);
LPWSTR KHCURLGetErrorMessageW(DWORD errorCode, const LPWSTR functionName);
void KHCURLDialogErrorA(CURLcode errorCode, const LPSTR functionName);
void KHCURLDialogErrorW(CURLcode errorCode, const LPWSTR functionName);
void KHCURLConsoleErrorA(CURLcode errorCode, const LPSTR functionName);
void KHCURLConsoleErrorW(CURLcode errorCode, const LPWSTR functionName);
#ifdef __cplusplus
}
#endif
