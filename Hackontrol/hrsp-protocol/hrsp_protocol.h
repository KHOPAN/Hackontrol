#pragma once

#include <Windows.h>

#define HRSP_PROTOCOL_VERSION       1
#define HRSP_PROTOCOL_VERSION_MINOR 0

typedef enum {
	HRSP_ERROR_SUCCESS = 0,
	HRSP_ERROR_UNKNOWN_ERROR,
	HRSP_ERROR_INVALID_FUNCTION_PARAMETER
} HRSPPROTOCOLERRORCODE;

typedef struct {
	BOOL win32Error;
	LPCWSTR functionName;
	HRSPPROTOCOLERRORCODE errorCode;
	DWORD win32ErrorCode;
} HRSPPROTOCOLERROR, *PHRSPPROTOCOLERROR;

#ifdef __cplusplus
extern "C" {
#endif

LPCWSTR HRSPGetErrorCode(const HRSPPROTOCOLERRORCODE code);
BOOL HRSPClientHandshake(const SOCKET socket, const PHRSPPROTOCOLERROR error);
BOOL HRSPServerHandshake(const SOCKET socket, const PHRSPPROTOCOLERROR error);

#ifdef __cplusplus
}
#endif
