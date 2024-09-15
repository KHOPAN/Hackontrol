#pragma once

#include <Windows.h>

typedef void(__stdcall* HRSPCLIENTCALLBACK) (LPVOID parameter);

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
	LPVOID parameter;
	HRSPCLIENTCALLBACK callbackConnected;
} HRSPCLIENTINPUT, *PHRSPCLIENTINPUT;

typedef struct {
	HRSPCLIENTERRORTYPE type;
	LPCWSTR function;
	DWORD code;
} HRSPCLIENTERROR, *PHRSPCLIENTERROR;

#ifdef __cplusplus
extern "C" {
#endif

LPWSTR HRSPClientGetErrorMessage(const LPCWSTR functionName, const PHRSPCLIENTERROR error);
LPCWSTR HRSPClientGetErrorCode(const HRSPCLIENTERRORCODE code);
BOOL HRSPClientConnectToServer(const LPCWSTR address, const LPCWSTR port, const PHRSPCLIENTINPUT input, const PHRSPCLIENTERROR error);

#ifdef __cplusplus
}
#endif
