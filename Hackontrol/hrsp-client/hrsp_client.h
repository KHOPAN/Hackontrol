#pragma once

#include <Windows.h>

typedef enum {
	HRSP_CLIENT_ERROR_TYPE_CLIENT = 0,
	HRSP_CLIENT_ERROR_TYPE_HRSP,
	HRSP_CLIENT_ERROR_TYPE_WIN32
} HRSPCLIENTERRORTYPE;

typedef enum {
	HRSP_CLIENT_ERROR_SUCCESS = 0,
	HRSP_CLIENT_ERROR_UNKNOWN_ERROR,
	HRSP_CLIENT_ERROR_CANNOT_CONNECT_SERVER
} HRSPCLIENTERRORCODE;

typedef struct {
	HRSPCLIENTERRORTYPE type;
	LPCWSTR function;
	DWORD code;
} HRSPCLIENTERROR, *PHRSPCLIENTERROR;

#ifdef __cplusplus
extern "C" {
#endif

LPCWSTR HRSPClientGetErrorCode(const HRSPCLIENTERRORCODE code);
LPWSTR HRSPClientGetErrorMessage(const LPCWSTR functionName, const PHRSPCLIENTERROR error);
BOOL HRSPClientConnectToServer(const LPCSTR address, const LPCSTR port, const PHRSPCLIENTERROR error);

#ifdef __cplusplus
}
#endif
