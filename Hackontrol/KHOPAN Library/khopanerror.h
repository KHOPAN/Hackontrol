#pragma once

#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif

LPSTR KHGetWin32ErrorMessageA(DWORD errorCode, const LPSTR functionName);
LPWSTR KHGetWin32ErrorMessageW(DWORD errorCode, const LPWSTR functionName);

void KHWin32DialogErrorA(DWORD errorCode, const LPSTR functionName);
void KHWin32DialogErrorW(DWORD errorCode, const LPWSTR functionName);

void KHWin32ConsoleErrorA(DWORD errorCode, const LPSTR functionName);
void KHWin32ConsoleErrorW(DWORD errorCode, const LPWSTR functionName);

DWORD KHDecodeHRESULTError(HRESULT result);

#ifdef __cplusplus
}
#endif
