#pragma once

#include <Windows.h>

#define CURL_STATICLIB
#include <curl/curl.h>

void curlError(CURLcode, const wchar_t*);
void dialogError(DWORD, const wchar_t*);
