#pragma once

#include <Windows.h>

#define FILE_CMD        L"cmd.exe"
#define FILE_RUNDLL32   L"rundll32.exe"
#define FOLDER_SYSTEM32 L"System32"

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
BOOL KHWin32StartProcessA(const LPSTR filePath, const LPSTR argument, BOOL wait);
BOOL KHWin32StartProcessW(const LPWSTR filePath, const LPWSTR argument, BOOL wait);
LPSTR KHWin32GetRundll32FileA();
LPWSTR KHWin32GetRundll32FileW();
BOOL KHWin32StartDynamicLibraryA(const LPSTR filePath, const LPSTR functionName, const LPSTR argument);
BOOL KHWin32StartDynamicLibraryW(const LPWSTR filePath, const LPWSTR functionName, const LPWSTR argument);
LPSTR KHWin32GetCmdFileA();
LPWSTR KHWin32GetCmdFileW();
BOOL KHWin32ExecuteCommandA(const LPSTR command, BOOL wait);
BOOL KHWin32ExecuteCommandW(const LPWSTR command, BOOL wait);
BOOL KHWin32ExecuteRundll32FunctionA(const LPSTR moduleName, const LPSTR functionName, LPSTR argument, BOOL threaded);
BOOL KHWin32ExecuteRundll32FunctionW(const LPWSTR moduleName, const LPSTR functionName, LPSTR argument, BOOL threaded);
DWORD KHWin32DecodeHRESULTError(HRESULT result);
#ifdef __cplusplus
}
#endif

#ifdef UNICODE
#define KHWin32GetWindowsDirectory     KHWin32GetWindowsDirectoryW
#define KHWin32GetErrorMessage         KHWin32GetErrorMessageW
#define KHWin32DialogError             KHWin32DialogErrorW
#define KHWin32ConsoleError            KHWin32ConsoleErrorW
#define KHWin32StartProcess            KHWin32StartProcessW
#define KHWin32GetRundll32File         KHWin32GetRundll32FileW
#define KHWin32StartDynamicLibrary     KHWin32StartDynamicLibraryW
#define KHWin32GetCmdFile              KHWin32GetCmdFileW
#define KHWin32ExecuteCommand          KHWin32ExecuteCommandW
#define KHWin32ExecuteRundll32Function KHWin32ExecuteRundll32FunctionW
#else
#define KHWin32GetWindowsDirectory     KHWin32GetWindowsDirectoryA
#define KHWin32GetErrorMessage         KHWin32GetErrorMessageA
#define KHWin32DialogError             KHWin32DialogErrorA
#define KHWin32ConsoleError            KHWin32ConsoleErrorA
#define KHWin32StartProcess            KHWin32StartProcessA
#define KHWin32GetRundll32File         KHWin32GetRundll32FileA
#define KHWin32StartDynamicLibrary     KHWin32StartDynamicLibraryA
#define KHWin32GetCmdFile              KHWin32GetCmdFileA
#define KHWin32ExecuteCommand          KHWin32ExecuteCommandA
#define KHWin32ExecuteRundll32Function KHWin32ExecuteRundll32FunctionA
#endif
