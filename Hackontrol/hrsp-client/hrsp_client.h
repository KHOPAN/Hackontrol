#pragma once

#include <hrsp.h>

typedef void(__stdcall* HRSPCLIENTCALLBACK) (const LPVOID parameter);

typedef struct {
	LPVOID parameter;
	HRSPCLIENTCALLBACK callbackConnected;
} HRSPCLIENTINPUT, *PHRSPCLIENTINPUT;

typedef enum {
	HRSP_CLIENT_CONNECT_OK
} HRSPCLIENTCONNECTSTATUS;

#ifdef __cplusplus
extern "C" {
#endif
//BOOL HRSPClientConnectToServer(const LPCWSTR address, const LPCWSTR port, const PHRSPCLIENTINPUT input, const PKHOPANERROR error);
HRSPCLIENTCONNECTSTATUS HRSPClientConnect(const LPCWSTR address, const LPCWSTR port);
#ifdef __cplusplus
}
#endif
