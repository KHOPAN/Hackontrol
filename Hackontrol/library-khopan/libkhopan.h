#pragma once

#include <stdio.h>
#include <Windows.h>

#define FILE_CMD        L"cmd.exe"
#define FILE_RUNDLL32   L"rundll32.exe"
#define FOLDER_SYSTEM32 L"System32"

#define KHOPAN_ALLOCATE(size)     HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size)
#define KHOPAN_DEALLOCATE(buffer) HeapFree(GetProcessHeap(),0,buffer)

#define ERROR_FACILITY_HRESULT 0x0000
#define ERROR_FACILITY_COMMON  0x0001

#define KHOPANERRORMESSAGE_HRESULT(result, functionName) do{KHOPANERROR __temporaryError__;__temporaryError__.facility=ERROR_FACILITY_HRESULT;__temporaryError__.code=(HRESULT)(result);__temporaryError__.function=(LPCWSTR)(functionName);LPWSTR __temporaryMessage__=KHOPANGetErrorMessage(&__temporaryError__);if(__temporaryMessage__){MessageBoxW(NULL,__temporaryMessage__,L"Error",MB_OK|MB_DEFBUTTON1|MB_ICONERROR|MB_SYSTEMMODAL);KHOPAN_DEALLOCATE(__temporaryMessage__);}}while(0)
#define KHOPANERRORMESSAGE_WIN32(code, function)         KHOPANERRORMESSAGE_HRESULT(HRESULT_FROM_WIN32((unsigned long)(code)),function)
#define KHOPANERRORMESSAGE_NTSTATUS(code, function)      KHOPANERRORMESSAGE_HRESULT((code)|0x10000000,function)

#define KHOPANERRORCONSOLE_HRESULT(result, functionName) do{KHOPANERROR __temporaryError__;__temporaryError__.facility=ERROR_FACILITY_HRESULT;__temporaryError__.code=(HRESULT)(result);__temporaryError__.function=(LPCWSTR)(functionName);LPWSTR __temporaryMessage__=KHOPANGetErrorMessage(&__temporaryError__);if(__temporaryMessage__){_putws(__temporaryMessage__);KHOPAN_DEALLOCATE(__temporaryMessage__);}}while(0)
#define KHOPANERRORCONSOLE_WIN32(code, function)         KHOPANERRORCONSOLE_HRESULT(HRESULT_FROM_WIN32((unsigned long)(code)),function)
#define KHOPANERRORCONSOLE_NTSTATUS(code, function)      KHOPANERRORCONSOLE_HRESULT((code)|0x10000000,function)

#define KHOPANLASTERRORMESSAGE_WIN32(function) KHOPANERRORMESSAGE_WIN32(GetLastError(),function)
#define KHOPANLASTERRORMESSAGE_WSA(function)   KHOPANERRORMESSAGE_WIN32(WSAGetLastError(),function)
#define KHOPANLASTERRORCONSOLE_WIN32(function) KHOPANERRORCONSOLE_WIN32(GetLastError(),function)
#define KHOPANLASTERRORCONSOLE_WSA(function)   KHOPANERRORCONSOLE_WIN32(WSAGetLastError(),function)

enum ERRORFACLIITYCOMMON {
	ERROR_COMMON_SUCCESS,
	ERROR_COMMON_UNDEFINED,
	ERROR_COMMON_FUNCTION_FAILED,
	ERROR_COMMON_INVALID_PARAMETER
};

typedef struct {
	UINT facility;
	ULONG code;
	LPCWSTR function;
} KHOPANERROR, *PKHOPANERROR;

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
LPWSTR KHOPANStringDuplicate(const LPCWSTR text);

LPWSTR KHOPANGetErrorMessage(const PKHOPANERROR error);
#ifdef __cplusplus
}
#endif

#define KHOPAN_ALLOCATE_ERROR 0
#define KHOPAN_ALLOCATE_FAILED(x) FALSE
