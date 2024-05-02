#pragma once

#include <Windows.h>

LPSTR KHGetWin32ErrorMessageA(DWORD errorCode, const LPSTR functionName);
LPWSTR KHGetWin32ErrorMessageW(DWORD errorCode, const LPWSTR functionName);

void KHWin32DialogErrorA(DWORD errorCode, const LPSTR functionName);
void KHWin32DialogErrorW(DWORD errorCode, const LPWSTR functionName);

void KHWin32ConsoleErrorA(DWORD errorCode, const LPSTR functionName);
void KHWin32ConsoleErrorW(DWORD errorCode, const LPWSTR functionName);

unsigned long KHDecodeHRESULTError(HRESULT result);
