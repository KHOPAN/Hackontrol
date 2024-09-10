#pragma once

#include <Windows.h>

#define HRSP_PROTOCOL_VERSION       1
#define HRSP_PROTOCOL_VERSION_MINOR 0
#define HRSP_PROTOCOL_PORT          42485
#define HRSP_PROTOCOL_PORT_STRING   "42485"

typedef enum {
	HRSP_ERROR_SUCCESS = 0,
	HRSP_ERROR_UNKNOWN_ERROR,
	HRSP_ERROR_INVALID_FUNCTION_PARAMETER,
	HRSP_ERROR_INVALID_MAGIC,
	HRSP_ERROR_UNSUPPORTED_VERSION
} HRSPERRORCODE;

typedef struct {
	BOOL win32;
	LPCWSTR function;
	DWORD code;
} HRSPERROR, *PHRSPERROR;

typedef struct {
	BYTE placeholder;
} HRSPDATA, *PHRSPDATA;

#ifdef __cplusplus
extern "C" {
#endif

LPCWSTR HRSPGetErrorCode(const HRSPERRORCODE code);
LPWSTR HRSPGetErrorMessage(const LPCWSTR functionName, const PHRSPERROR error);

#ifdef __cplusplus
}
#endif
