#pragma once

#include <stdio.h>
#include <Windows.h>

#define FILE_CMD        L"cmd.exe"
#define FILE_RUNDLL32   L"rundll32.exe"
#define FOLDER_SYSTEM32 L"System32"

#define KHWIN32_MESSAGE(code, function)    KHInternal_ErrorMessage((DWORD)(code),(LPWSTR)(function),__FILEW__,__LINE__,FALSE)
#define KHNTSTATUS_MESSAGE(code, function) KHInternal_ErrorMessage((DWORD)(code),(LPWSTR)(function),__FILEW__,__LINE__,TRUE)

#define KHWIN32_ERROR(code, function)            do{LPWSTR __temporary_message__=KHWIN32_MESSAGE(code,function);if(__temporary_message__){MessageBoxW(NULL,__temporary_message__,L"Error",MB_OK|MB_DEFBUTTON1|MB_ICONERROR|MB_SYSTEMMODAL);LocalFree(__temporary_message__);}}while(0)
#define KHWIN32_ERROR_CONSOLE(code, function)    do{LPWSTR __temporary_message__=KHWIN32_MESSAGE(code,function);if(__temporary_message__){printf("%ws\n",__temporary_message__);LocalFree(__temporary_message__);}}while(0)
#define KHNTSTATUS_ERROR(code, function)         do{LPWSTR __temporary_message__=KHNTSTATUS_MESSAGE(code,function);if(__temporary_message__){MessageBoxW(NULL,__temporary_message__,L"Error",MB_OK|MB_DEFBUTTON1|MB_ICONERROR|MB_SYSTEMMODAL);LocalFree(__temporary_message__);}}while(0)
#define KHNTSTATUS_ERROR_CONSOLE(code, function) do{LPWSTR __temporary_message__=KHNTSTATUS_MESSAGE(code,function);if(__temporary_message__){printf("%ws\n",__temporary_message__);LocalFree(__temporary_message__);}}while(0)

#define KHWIN32_LAST_ERROR(function)             KHWIN32_ERROR(GetLastError(),function)
#define KHWIN32_LAST_ERROR_CONSOLE(function)     KHWIN32_ERROR_CONSOLE(GetLastError(),function)
#define KHWIN32_LAST_WSA_ERROR(function)         KHWIN32_ERROR(WSAGetLastError(),function)
#define KHWIN32_LAST_WSA_ERROR_CONSOLE(function) KHWIN32_ERROR_CONSOLE(WSAGetLastError(),function)

#define KHOPANERRORMESSAGE_WIN32(code, function)    do{LPWSTR __temporaryMessage__=KHOPANInternalGetErrorMessage(code,function,TRUE);if(__temporaryMessage__){MessageBoxW(NULL,__temporaryMessage__,L"Error",MB_OK|MB_DEFBUTTON1|MB_ICONERROR|MB_SYSTEMMODAL);LocalFree(__temporaryMessage__);}}while(0)
#define KHOPANERRORMESSAGE_HRESULT(code, function)  do{LPWSTR __temporaryMessage__=KHOPANInternalGetErrorMessage((code)==S_OK?ERROR_SUCCESS:(HRESULT)(((HRESULT)code)&0xFFFF0000)==MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,0)?HRESULT_CODE((HRESULT)code):ERROR_FUNCTION_FAILED,function,TRUE);if(__temporaryMessage__){MessageBoxW(NULL,__temporaryMessage__,L"Error",MB_OK|MB_DEFBUTTON1|MB_ICONERROR|MB_SYSTEMMODAL);LocalFree(__temporaryMessage__);}}while(0)
#define KHOPANERRORMESSAGE_NTSTATUS(code, function) do{LPWSTR __temporaryMessage__=KHOPANInternalGetErrorMessage(code,function,FALSE);if(__temporaryMessage__){MessageBoxW(NULL,__temporaryMessage__,L"Error",MB_OK|MB_DEFBUTTON1|MB_ICONERROR|MB_SYSTEMMODAL);LocalFree(__temporaryMessage__);}}while(0)

#define KHOPANERRORCONSOLE_WIN32(code, function)    do{LPWSTR __temporaryMessage__=KHOPANInternalGetErrorMessage(code,function,TRUE);if(__temporaryMessage__){_putws(__temporaryMessage__);LocalFree(__temporaryMessage__);}}while(0)
#define KHOPANERRORCONSOLE_HRESULT(code, function)  do{LPWSTR __temporaryMessage__=KHOPANInternalGetErrorMessage((code)==S_OK?ERROR_SUCCESS:(HRESULT)(((HRESULT)code)&0xFFFF0000)==MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,0)?HRESULT_CODE((HRESULT)code):ERROR_FUNCTION_FAILED,function,TRUE);if(__temporaryMessage__){_putws(__temporaryMessage__);LocalFree(__temporaryMessage__);}}while(0)
#define KHOPANERRORCONSOLE_NTSTATUS(code, function) do{LPWSTR __temporaryMessage__=KHOPANInternalGetErrorMessage(code,function,FALSE);if(__temporaryMessage__){_putws(__temporaryMessage__);LocalFree(__temporaryMessage__);}}while(0)

#define KHOPANLASTERRORMESSAGE_WIN32(function) KHOPANERRORMESSAGE_WIN32(GetLastError(),function)
#define KHOPANLASTERRORMESSAGE_WSA(function)   KHOPANERRORMESSAGE_WIN32(WSAGetLastError(),function)

#define KHOPANLASTERRORCONSOLE_WIN32(function) KHOPANERRORCONSOLE_WIN32(GetLastError(),function)
#define KHOPANLASTERRORCONSOLE_WSA(function)   KHOPANERRORCONSOLE_WIN32(WSAGetLastError(),function)

#ifdef __cplusplus
extern "C" {
#endif
LPWSTR KHOPANGetErrorMessageWin32(const DWORD code, const LPCWSTR function);
LPWSTR KHOPANGetErrorMessageHRESULT(const HRESULT code, const LPCWSTR function);
LPWSTR KHOPANGetErrorMessageNTSTATUS(const NTSTATUS code, const LPCWSTR function);

LPWSTR KHOPANInternalGetErrorMessage(const DWORD code, const LPCWSTR function, const BOOL win32);

LPSTR KHWin32GetWindowsDirectoryA();
LPWSTR KHWin32GetWindowsDirectoryW();
LPWSTR KHInternal_ErrorMessage(const DWORD errorCode, const LPCWSTR functionName, const LPCWSTR fileName, const UINT lineNumber, const BOOL specialError);
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
BOOL KHWin32EnablePrivilegeA(const LPSTR privilege);
BOOL KHWin32EnablePrivilegeW(const LPWSTR privilege);
BOOL KHWin32RegistrySetStringValueA(const HKEY key, const LPSTR valueName, const LPSTR value);
BOOL KHWin32RegistrySetStringValueW(const HKEY key, const LPWSTR valueName, const LPWSTR value);
#ifdef __cplusplus
}
#endif
