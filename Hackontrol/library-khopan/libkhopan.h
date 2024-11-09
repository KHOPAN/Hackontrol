#pragma once

#include <stdio.h>
#include <Windows.h>

#define FILE_CMD        L"cmd.exe"
#define FILE_RUNDLL32   L"rundll32.exe"
#define FOLDER_SYSTEM32 L"System32"

#define KHOPAN_ALLOCATE(size)     HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size)
#define KHOPAN_DEALLOCATE(buffer) HeapFree(GetProcessHeap(),0,buffer)

#define ERROR_FACILITY_WIN32    0x0000
#define ERROR_FACILITY_HRESULT  0x0001
#define ERROR_FACILITY_NTSTATUS 0x0002
#define ERROR_FACILITY_CURL     0x0003
#define ERROR_FACILITY_COMMON   0x0004

#define KHOPANERRORCONSOLE_KHOPAN(code)                    do{LPWSTR __temporaryMessage__=KHOPANGetErrorMessage((PKHOPANERROR)(&code),KHOPAN_ERROR_DECODER);if(__temporaryMessage__){_putws(__temporaryMessage__);KHOPAN_DEALLOCATE(__temporaryMessage__);}}while(0)
#define KHOPANERRORCONSOLE_WIN32(codeError, sourceName)    do{KHOPANERROR __temporaryError__={0};__temporaryError__.facility=ERROR_FACILITY_WIN32;__temporaryError__.code=(ULONG)(codeError);__temporaryError__.source=(LPCWSTR)(sourceName);LPWSTR __temporaryMessage__=KHOPANGetErrorMessage(&__temporaryError__,KHOPAN_ERROR_DECODER);if(__temporaryMessage__){_putws(__temporaryMessage__);KHOPAN_DEALLOCATE(__temporaryMessage__);}}while(0)
#define KHOPANERRORCONSOLE_HRESULT(codeError, sourceName)  do{KHOPANERROR __temporaryError__={0};__temporaryError__.facility=ERROR_FACILITY_HRESULT;__temporaryError__.code=(HRESULT)(codeError);__temporaryError__.source=(LPCWSTR)(sourceName);LPWSTR __temporaryMessage__=KHOPANGetErrorMessage(&__temporaryError__,KHOPAN_ERROR_DECODER);if(__temporaryMessage__){_putws(__temporaryMessage__);KHOPAN_DEALLOCATE(__temporaryMessage__);}}while(0)
#define KHOPANERRORCONSOLE_NTSTATUS(codeError, sourceName) do{KHOPANERROR __temporaryError__={0};__temporaryError__.facility=ERROR_FACILITY_NTSTATUS;__temporaryError__.code=(NTSTATUS)(codeError);__temporaryError__.source=(LPCWSTR)(sourceName);LPWSTR __temporaryMessage__=KHOPANGetErrorMessage(&__temporaryError__,KHOPAN_ERROR_DECODER);if(__temporaryMessage__){_putws(__temporaryMessage__);KHOPAN_DEALLOCATE(__temporaryMessage__);}}while(0)
#define KHOPANERRORCONSOLE_CURL(codeError, sourceName)     do{KHOPANERROR __temporaryError__={0};__temporaryError__.facility=ERROR_FACILITY_CURL;__temporaryError__.code=(ULONG)(codeError);__temporaryError__.source=(LPCWSTR)(sourceName);LPWSTR __temporaryMessage__=KHOPANGetErrorMessage(&__temporaryError__,KHOPAN_ERROR_DECODER);if(__temporaryMessage__){_putws(__temporaryMessage__);KHOPAN_DEALLOCATE(__temporaryMessage__);}}while(0)

#define KHOPANERRORMESSAGE_KHOPAN(code)                    do{LPWSTR __temporaryMessage__=KHOPANGetErrorMessage((PKHOPANERROR)(&code),KHOPAN_ERROR_DECODER);if(__temporaryMessage__){MessageBoxW(NULL,__temporaryMessage__,L"Error",MB_OK|MB_DEFBUTTON1|MB_ICONERROR|MB_SYSTEMMODAL);KHOPAN_DEALLOCATE(__temporaryMessage__);}}while(0)
#define KHOPANERRORMESSAGE_WIN32(codeError, sourceName)    do{KHOPANERROR __temporaryError__={0};__temporaryError__.facility=ERROR_FACILITY_WIN32;__temporaryError__.code=(ULONG)(codeError);__temporaryError__.source=(LPCWSTR)(sourceName);LPWSTR __temporaryMessage__=KHOPANGetErrorMessage(&__temporaryError__,KHOPAN_ERROR_DECODER);if(__temporaryMessage__){MessageBoxW(NULL,__temporaryMessage__,L"Error",MB_OK|MB_DEFBUTTON1|MB_ICONERROR|MB_SYSTEMMODAL);KHOPAN_DEALLOCATE(__temporaryMessage__);}}while(0)
#define KHOPANERRORMESSAGE_HRESULT(codeError, sourceName)  do{KHOPANERROR __temporaryError__={0};__temporaryError__.facility=ERROR_FACILITY_HRESULT;__temporaryError__.code=(HRESULT)(codeError);__temporaryError__.source=(LPCWSTR)(sourceName);LPWSTR __temporaryMessage__=KHOPANGetErrorMessage(&__temporaryError__,KHOPAN_ERROR_DECODER);if(__temporaryMessage__){MessageBoxW(NULL,__temporaryMessage__,L"Error",MB_OK|MB_DEFBUTTON1|MB_ICONERROR|MB_SYSTEMMODAL);KHOPAN_DEALLOCATE(__temporaryMessage__);}}while(0)
#define KHOPANERRORMESSAGE_NTSTATUS(codeError, sourceName) do{KHOPANERROR __temporaryError__={0};__temporaryError__.facility=ERROR_FACILITY_NTSTATUS;__temporaryError__.code=(NTSTATUS)(codeError);__temporaryError__.source=(LPCWSTR)(sourceName);LPWSTR __temporaryMessage__=KHOPANGetErrorMessage(&__temporaryError__,KHOPAN_ERROR_DECODER);if(__temporaryMessage__){MessageBoxW(NULL,__temporaryMessage__,L"Error",MB_OK|MB_DEFBUTTON1|MB_ICONERROR|MB_SYSTEMMODAL);KHOPAN_DEALLOCATE(__temporaryMessage__);}}while(0)
#define KHOPANERRORMESSAGE_CURL(codeError, sourceName)     do{KHOPANERROR __temporaryError__={0};__temporaryError__.facility=ERROR_FACILITY_CURL;__temporaryError__.code=(ULONG)(codeError);__temporaryError__.source=(LPCWSTR)(sourceName);LPWSTR __temporaryMessage__=KHOPANGetErrorMessage(&__temporaryError__,KHOPAN_ERROR_DECODER);if(__temporaryMessage__){MessageBoxW(NULL,__temporaryMessage__,L"Error",MB_OK|MB_DEFBUTTON1|MB_ICONERROR|MB_SYSTEMMODAL);KHOPAN_DEALLOCATE(__temporaryMessage__);}}while(0)

#define KHOPANLASTERRORCONSOLE_WIN32(source) KHOPANERRORCONSOLE_WIN32(GetLastError(),source)
#define KHOPANLASTERRORCONSOLE_WSA(source)   KHOPANERRORCONSOLE_WIN32(WSAGetLastError(),source)
#define KHOPANLASTERRORMESSAGE_WIN32(source) KHOPANERRORMESSAGE_WIN32(GetLastError(),source)
#define KHOPANLASTERRORMESSAGE_WSA(source)   KHOPANERRORMESSAGE_WIN32(WSAGetLastError(),source)

#define KHOPAN_ERROR_DECODER KHOPANErrorCommonDecoder

#define CURL_STATICLIB

enum ERRORFACLIITYCOMMON {
	ERROR_COMMON_SUCCESS,
	ERROR_COMMON_UNDEFINED,
	ERROR_COMMON_FUNCTION_FAILED,
	ERROR_COMMON_INVALID_PARAMETER,
	ERROR_COMMON_INDEX_OUT_OF_BOUNDS
};

typedef struct {
	UINT facility;
	ULONG code;
	LPCWSTR source;
	LPCWSTR function;
} KHOPANERROR, *PKHOPANERROR;

typedef LPCWSTR(__stdcall* KHOPANERRORDECODER) (const PKHOPANERROR error);

#ifdef __cplusplus
extern "C" {
#endif
LPCWSTR KHOPANErrorCommonDecoder(const PKHOPANERROR error);
LPWSTR KHOPANFormatMessage(const LPCWSTR format, ...);
LPSTR KHOPANFormatANSI(const LPCSTR format, ...);
LPWSTR KHOPANGetErrorMessage(const PKHOPANERROR error, const KHOPANERRORDECODER decoder);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
BOOL KHOPANEnablePrivilege(const LPCWSTR privilege, const PKHOPANERROR error);
BOOL KHOPANExecuteCommand(const LPCWSTR command, const BOOL block, const PKHOPANERROR error);
BOOL KHOPANExecuteDynamicLibrary(const LPCWSTR file, const LPCSTR function, const LPCSTR argument, const PKHOPANERROR error);
BOOL KHOPANExecuteProcess(const LPCWSTR file, const LPCWSTR argument, const BOOL block, const PKHOPANERROR error);
BOOL KHOPANExecuteRundll32Function(const LPWSTR file, const LPCSTR function, const LPCSTR argument, const BOOL block, const PKHOPANERROR error);
LPWSTR KHOPANFileGetCmd(const PKHOPANERROR error);
LPWSTR KHOPANFileGetRundll32(const PKHOPANERROR error);
LPWSTR KHOPANFolderGetWindows(const PKHOPANERROR error);
LPWSTR KHOPANStringDuplicate(const LPCWSTR text, const PKHOPANERROR error);
#ifdef __cplusplus
}
#endif

#define KHOPAN_ALLOCATE_ERROR 0
#define KHOPAN_ALLOCATE_FAILED(x) FALSE
