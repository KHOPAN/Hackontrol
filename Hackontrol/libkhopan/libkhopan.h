#pragma once

#include <stdio.h>
#include <Windows.h>

#define FILE_CMD        L"cmd.exe"
#define FILE_RUNDLL32   L"rundll32.exe"
#define FOLDER_SYSTEM32 L"System32"

#define KHOPANRAWERROR_WIN32(code, function)    KHOPANInternalGetErrorMessage(code,function,TRUE)
#define KHOPANRAWERROR_HRESULT(code, function)  KHOPANInternalGetErrorMessage((code)==S_OK?ERROR_SUCCESS:(HRESULT)(((HRESULT)code)&0xFFFF0000)==MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,0)?HRESULT_CODE((HRESULT)code):ERROR_FUNCTION_FAILED,function,TRUE)
#define KHOPANRAWERROR_NTSTATUS(code, function) KHOPANInternalGetErrorMessage(code,function,FALSE)

#define KHOPANERRORMESSAGE_WIN32(code, function)    do{LPWSTR __temporaryMessage__=KHOPANRAWERROR_WIN32(code,function);if(__temporaryMessage__){MessageBoxW(NULL,__temporaryMessage__,L"Error",MB_OK|MB_DEFBUTTON1|MB_ICONERROR|MB_SYSTEMMODAL);LocalFree(__temporaryMessage__);}}while(0)
#define KHOPANERRORMESSAGE_HRESULT(code, function)  do{LPWSTR __temporaryMessage__=KHOPANRAWERROR_HRESULT(code,function);if(__temporaryMessage__){MessageBoxW(NULL,__temporaryMessage__,L"Error",MB_OK|MB_DEFBUTTON1|MB_ICONERROR|MB_SYSTEMMODAL);LocalFree(__temporaryMessage__);}}while(0)
#define KHOPANERRORMESSAGE_NTSTATUS(code, function) do{LPWSTR __temporaryMessage__=KHOPANRAWERROR_NTSTATUS(code,function);if(__temporaryMessage__){MessageBoxW(NULL,__temporaryMessage__,L"Error",MB_OK|MB_DEFBUTTON1|MB_ICONERROR|MB_SYSTEMMODAL);LocalFree(__temporaryMessage__);}}while(0)

#define KHOPANERRORCONSOLE_WIN32(code, function)    do{LPWSTR __temporaryMessage__=KHOPANRAWERROR_WIN32(code,function);if(__temporaryMessage__){_putws(__temporaryMessage__);LocalFree(__temporaryMessage__);}}while(0)
#define KHOPANERRORCONSOLE_HRESULT(code, function)  do{LPWSTR __temporaryMessage__=KHOPANRAWERROR_HRESULT(code,function);if(__temporaryMessage__){_putws(__temporaryMessage__);LocalFree(__temporaryMessage__);}}while(0)
#define KHOPANERRORCONSOLE_NTSTATUS(code, function) do{LPWSTR __temporaryMessage__=KHOPANRAWERROR_NTSTATUS(code,function);if(__temporaryMessage__){_putws(__temporaryMessage__);LocalFree(__temporaryMessage__);}}while(0)

#define KHOPANLASTERRORMESSAGE_WIN32(function) KHOPANERRORMESSAGE_WIN32(GetLastError(),function)
#define KHOPANLASTERRORMESSAGE_WSA(function)   KHOPANERRORMESSAGE_WIN32(WSAGetLastError(),function)

#define KHOPANLASTERRORCONSOLE_WIN32(function) KHOPANERRORCONSOLE_WIN32(GetLastError(),function)
#define KHOPANLASTERRORCONSOLE_WSA(function)   KHOPANERRORCONSOLE_WIN32(WSAGetLastError(),function)

#ifdef __cplusplus
extern "C" {
#endif
BOOL KHOPANEnablePrivilege(const LPWSTR privilege);
LPWSTR KHOPANFileGetCmd();
LPWSTR KHOPANFileGetRundll32();
LPWSTR KHOPANFolderGetWindows();
LPWSTR KHOPANFormatMessage(const LPWSTR format, ...);
LPWSTR KHOPANInternalGetErrorMessage(const DWORD code, const LPCWSTR function, const BOOL win32);

BOOL KHWin32StartProcessA(const LPSTR filePath, const LPSTR argument, BOOL wait);
BOOL KHWin32StartProcessW(const LPWSTR filePath, const LPWSTR argument, BOOL wait);

BOOL KHWin32StartDynamicLibraryA(const LPSTR filePath, const LPSTR functionName, const LPSTR argument);
BOOL KHWin32StartDynamicLibraryW(const LPWSTR filePath, const LPWSTR functionName, const LPWSTR argument);
BOOL KHWin32ExecuteCommandA(const LPSTR command, BOOL wait);
BOOL KHWin32ExecuteCommandW(const LPWSTR command, BOOL wait);
BOOL KHWin32ExecuteRundll32FunctionA(const LPSTR moduleName, const LPSTR functionName, LPSTR argument, BOOL threaded);
BOOL KHWin32ExecuteRundll32FunctionW(const LPWSTR moduleName, const LPSTR functionName, LPSTR argument, BOOL threaded);
BOOL KHWin32RegistrySetStringValueA(const HKEY key, const LPSTR valueName, const LPSTR value);
BOOL KHWin32RegistrySetStringValueW(const HKEY key, const LPWSTR valueName, const LPWSTR value);
#ifdef __cplusplus
}
#endif
