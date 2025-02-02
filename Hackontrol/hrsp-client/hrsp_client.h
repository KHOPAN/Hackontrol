#pragma once

#include <hrsp.h>

/*typedef void(__stdcall* HRSPCLIENTCALLBACK) (const LPVOID parameter);

typedef struct {
	LPVOID parameter;
	HRSPCLIENTCALLBACK callbackConnected;
} HRSPCLIENTINPUT, *PHRSPCLIENTINPUT;*/

typedef struct {
	LPCWSTR address;
	LPCWSTR port;

	struct {
		BOOLEAN wsaInitialized : 1;
		BOOLEAN wsaNoCleanup : 1;
	};
} HRPSCLIENTPARAMETER, *PHRPSCLIENTPARAMETER;

typedef enum {
	HRSP_CLIENT_OK,
	HRSP_CLIENT_NULL_PARAMETER,
	HRSP_CLIENT_WSA_INITIALIZATION_FAILED,
	HRSP_CLIENT_HOST_TRANSLATION_FAILED,
	HRSP_CLIENT_CONNECTION_FAILED,
	HRPS_CLIENT_BCRYPT_CANNOT_OPEN_ALGORITHM,
	HRSP_CLIENT_MEMORY_ALLOCATION_FAILED,
	HRSP_CLIENT_WSA_CLEANUP_FAILED
} HRSPCLIENTSTATUS;

#ifdef __cplusplus
extern "C" {
#endif
//BOOL HRSPClientConnectToServer(const LPCWSTR address, const LPCWSTR port, const PHRSPCLIENTINPUT input, const PKHOPANERROR error);
HRSPCLIENTSTATUS HRSPClientConnect(const PHRPSCLIENTPARAMETER parameter);
#ifdef __cplusplus
}
#endif
