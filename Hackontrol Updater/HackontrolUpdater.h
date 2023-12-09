#pragma once

#define CURL_STATICLIB
#include <curl/curl.h>

#ifdef __cplusplus
extern "C" {
#endif
CURL* HU_InitializeCURL();
const char* HU_GetVersionFile(CURL*);
BOOL HU_IsFileExists(const wchar_t*);
const char* HU_Hash(BYTE*, size_t);
#ifdef __cplusplus
}
#endif
