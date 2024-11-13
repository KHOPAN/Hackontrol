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

#define KHOPANERRORCONSOLE_KHOPAN(error)          InternalKHOPANErrorKHOPAN(&error,(KHOPAN_ERROR_DECODER),TRUE)
#define KHOPANERRORCONSOLE_WIN32(code, source)    InternalKHOPANError(ERROR_FACILITY_WIN32,(code),(source),(KHOPAN_ERROR_DECODER),TRUE)
#define KHOPANERRORCONSOLE_HRESULT(code, source)  InternalKHOPANError(ERROR_FACILITY_HRESULT,(code),(source),(KHOPAN_ERROR_DECODER),TRUE)
#define KHOPANERRORCONSOLE_NTSTATUS(code, source) InternalKHOPANError(ERROR_FACILITY_NTSTATUS,(code),(source),(KHOPAN_ERROR_DECODER),TRUE)
#define KHOPANERRORCONSOLE_CURL(code, source)     InternalKHOPANError(ERROR_FACILITY_CURL,(code),(source),(KHOPAN_ERROR_DECODER),TRUE)
#define KHOPANERRORCONSOLE_COMMON(code, source)   InternalKHOPANError(ERROR_FACILITY_COMMON,(code),(source),(KHOPAN_ERROR_DECODER),TRUE)

#define KHOPANERRORMESSAGE_KHOPAN(error)          InternalKHOPANErrorKHOPAN(&error,(KHOPAN_ERROR_DECODER),FALSE)
#define KHOPANERRORMESSAGE_WIN32(code, source)    InternalKHOPANError(ERROR_FACILITY_WIN32,(code),(source),(KHOPAN_ERROR_DECODER),FALSE)
#define KHOPANERRORMESSAGE_HRESULT(code, source)  InternalKHOPANError(ERROR_FACILITY_HRESULT,(code),(source),(KHOPAN_ERROR_DECODER),FALSE)
#define KHOPANERRORMESSAGE_NTSTATUS(code, source) InternalKHOPANError(ERROR_FACILITY_NTSTATUS,(code),(source),(KHOPAN_ERROR_DECODER),FALSE)
#define KHOPANERRORMESSAGE_CURL(code, source)     InternalKHOPANError(ERROR_FACILITY_CURL,(code),(source),(KHOPAN_ERROR_DECODER),FALSE)
#define KHOPANERRORMESSAGE_COMMON(code, source)   InternalKHOPANError(ERROR_FACILITY_COMMON,(code),(source),(KHOPAN_ERROR_DECODER),FALSE)

#define KHOPANLASTERRORCONSOLE_WIN32(source) KHOPANERRORCONSOLE_WIN32(GetLastError(),source)
#define KHOPANLASTERRORCONSOLE_WSA(source)   KHOPANERRORCONSOLE_WIN32(WSAGetLastError(),source)
#define KHOPANLASTERRORMESSAGE_WIN32(source) KHOPANERRORMESSAGE_WIN32(GetLastError(),source)
#define KHOPANLASTERRORMESSAGE_WSA(source)   KHOPANERRORMESSAGE_WIN32(WSAGetLastError(),source)

#define KHOPAN_ERROR_DECODER KHOPANErrorCommonDecoder

enum ERRORFACLIITYCOMMON {
	ERROR_COMMON_SUCCESS,
	ERROR_COMMON_UNDEFINED,
	ERROR_COMMON_FUNCTION_FAILED,
	ERROR_COMMON_INVALID_PARAMETER,
	ERROR_COMMON_ALLOCATION_FAILED,
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
void InternalKHOPANError(const UINT facility, const ULONG code, const LPCWSTR source, const KHOPANERRORDECODER decoder, const BOOL console);
void InternalKHOPANErrorKHOPAN(const PKHOPANERROR error, const KHOPANERRORDECODER decoder, const BOOL console);
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
LPWSTR KHOPANFileGetCmd();
LPWSTR KHOPANFileGetRundll32();
LPWSTR KHOPANFolderGetWindows();
LPWSTR KHOPANStringDuplicate(const LPCWSTR text);
BOOL KHOPANWriteFile(const LPCWSTR file, const LPVOID data, const size_t size, const PKHOPANERROR error);
#ifdef __cplusplus
}
#endif
