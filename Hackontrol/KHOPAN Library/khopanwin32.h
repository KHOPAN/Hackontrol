#pragma once

#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif
LPSTR KHWin32GetWindowsDirectoryA();
LPWSTR KHWin32GetWindowsDirectoryW();
LPSTR KHWin32GetErrorMessageA(DWORD errorCode, const LPSTR functionName);
LPWSTR KHWin32GetErrorMessageW(DWORD errorCode, const LPWSTR functionName);
void KHWin32DialogErrorA(DWORD errorCode, const LPSTR functionName);
void KHWin32DialogErrorW(DWORD errorCode, const LPWSTR functionName);
void KHWin32ConsoleErrorA(DWORD errorCode, const LPSTR functionName);
void KHWin32ConsoleErrorW(DWORD errorCode, const LPWSTR functionName);
DWORD KHWin32DecodeHRESULTError(HRESULT result);
#ifdef __cplusplus
}
#endif

#ifdef UNICODE
#define KHWin32GetWindowsDirectory KHWin32GetWindowsDirectoryW
#define KHWin32GetErrorMessage     KHWin32GetErrorMessageW
#define KHWin32DialogError         KHWin32DialogErrorW
#define KHWin32ConsoleError        KHWin32ConsoleErrorW
#else
#define KHWin32GetWindowsDirectory KHWin32GetWindowsDirectoryA
#define KHWin32GetErrorMessage     KHWin32GetErrorMessageA
#define KHWin32DialogError         KHWin32DialogErrorA
#define KHWin32ConsoleError        KHWin32ConsoleErrorA
#endif