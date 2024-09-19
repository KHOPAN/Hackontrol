#pragma once

#include <Windows.h>

#define CURL_STATICLIB
#include <curl/curl.h>

#define KHOPANRAWERROR_CURL(code, function) KHOPANCURLInternalGetErrorMessage(code,function)

#define KHOPANERRORMESSAGE_CURL(code, function) do{LPWSTR __temporaryMessage__=KHOPANRAWERROR_CURL(code,function);if(__temporaryMessage__){MessageBoxW(NULL,__temporaryMessage__,L"Error",MB_OK|MB_DEFBUTTON1|MB_ICONERROR|MB_SYSTEMMODAL);LocalFree(__temporaryMessage__);}}while(0)
#define KHOPANERRORCONSOLE_CURL(code, function) do{LPWSTR __temporaryMessage__=KHOPANRAWERROR_CURL(code,function);if(__temporaryMessage__){_putws(__temporaryMessage__);LocalFree(__temporaryMessage__);}}while(0)

#ifdef __cplusplus
extern "C" {
#endif
LPWSTR KHOPANCURLInternalGetErrorMessage(const CURLcode code, const LPCWSTR function);
#ifdef __cplusplus
}
#endif
