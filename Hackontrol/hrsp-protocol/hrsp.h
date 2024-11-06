#pragma once

#include <Windows.h>

#define HRSP_PROTOCOL_VERSION       1
#define HRSP_PROTOCOL_VERSION_MINOR 0
#define HRSP_PROTOCOL_PORT          42485
#define HRSP_PROTOCOL_PORT_STRING   L"42485"

#define HRSP_FACILITY_WIN32 0x00
#define HRSP_FACILITY_HRSP  0x01

typedef enum {
	HRSP_ERROR_SUCCESS,
	HRSP_ERROR_UNDEFINED,
	HRSP_ERROR_FUNCTION_FAILED,
	HRSP_ERROR_INVALID_PARAMETER,
	HRSP_ERROR_INVALID_MAGIC,
	HRSP_ERROR_UNSUPPORTED_VERSION,
	HRSP_ERROR_CONNECTION_CLOSED
} HRSPERRORCODE;

typedef struct {
	UINT facility;
	ULONG code;
	LPCWSTR function;
} HRSPERROR, *PHRSPERROR;

//typedef struct {
//	BYTE placeholder;
//} HRSPDATA, *PHRSPDATA;

#ifdef __cplusplus
extern "C" {
#endif
LPCWSTR HRSPGetErrorCode(const HRSPERRORCODE code);
LPWSTR HRSPGetErrorMessage(const LPCWSTR function, const PHRSPERROR error);
#ifdef __cplusplus
}
#endif
