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

#define KHOPAN_USE_HEAP

#ifdef KHOPAN_USE_HEAP
#define KHOPAN_ALLOCATE(size)         ((PBYTE)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(size_t)(size)))
#define KHOPAN_ALLOCATE_ERROR(buffer) (((PBYTE)(buffer))==NULL)
#define KHOPAN_ALLOCATE_WIN32_CODE    (ERROR_FUNCTION_FAILED)
#define KHOPAN_FREE(buffer)           (HeapFree(GetProcessHeap(),0,(PBYTE)(buffer)))
#else
#define KHOPAN_ALLOCATE(size)         ((PBYTE)LocalAlloc(LMEM_FIXED,(size_t)(size)))
#define KHOPAN_ALLOCATE_ERROR(buffer) (((PBYTE)(buffer))==NULL)
#define KHOPAN_ALLOCATE_WIN32_CODE    (GetLastError())
#define KHOPAN_FREE(buffer)           (LocalFree((PBYTE)(buffer)))
#endif

#ifdef __cplusplus
extern "C" {
#endif
BOOL KHOPANEnablePrivilege(const LPCWSTR privilege);
BOOL KHOPANExecuteCommand(const LPCWSTR command, const BOOL block);
BOOL KHOPANExecuteDynamicLibrary(const LPCWSTR file, const LPCSTR function, const LPCSTR argument);
BOOL KHOPANExecuteProcess(const LPCWSTR file, const LPCWSTR argument, const BOOL block);
BOOL KHOPANExecuteRundll32Function(const LPWSTR file, const LPCSTR function, const LPCSTR argument, const BOOL block);
LPWSTR KHOPANFileGetCmd();
LPWSTR KHOPANFileGetRundll32();
LPWSTR KHOPANFolderGetWindows();
LPWSTR KHOPANFormatMessage(const LPCWSTR format, ...);
LPSTR KHOPANFormatANSI(const LPCSTR format, ...);
LPWSTR KHOPANInternalGetErrorMessage(const DWORD code, const LPCWSTR function, const BOOL win32);
LPWSTR KHOPANStringDuplicate(const LPCWSTR text);
#ifdef __cplusplus
}
#endif
