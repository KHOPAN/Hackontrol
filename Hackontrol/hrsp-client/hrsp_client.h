#pragma once

#include <hrsp.h>

typedef void(__stdcall* HRSPCLIENTCALLBACK) (const LPVOID parameter);

typedef struct {
	LPVOID parameter;
	HRSPCLIENTCALLBACK callbackConnected;
} HRSPCLIENTINPUT, *PHRSPCLIENTINPUT;

typedef struct {
	LPCWSTR address;
	LPCWSTR port;
} HRPSCLIENTPARAMETER, *PHRPSCLIENTPARAMETER;

typedef enum {
	HRSP_CLIENT_OK,
	HRSP_CLIENT_NULL_PARAMETER
} HRSPCLIENTSTATUS;

#ifdef __cplusplus
extern "C" {
#endif
//BOOL HRSPClientConnectToServer(const LPCWSTR address, const LPCWSTR port, const PHRSPCLIENTINPUT input, const PKHOPANERROR error);
HRSPCLIENTSTATUS HRSPClientConnect(const PHRPSCLIENTPARAMETER parameter);
#ifdef __cplusplus
}
#endif
