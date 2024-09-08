#pragma once

#include <Windows.h>

typedef enum {
	HRSP_ERROR_INVALID_FUNCTION_PARAMETER = 1
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

BOOL HRSPClientHandshake(const SOCKET socket, const PHRSPPROTOCOLERROR error);
BOOL HRSPServerHandshake(const SOCKET socket, const PHRSPPROTOCOLERROR error);

#ifdef __cplusplus
}
#endif
