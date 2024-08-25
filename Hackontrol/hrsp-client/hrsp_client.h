#pragma once

#include <Windows.h>

typedef enum {
	REMOTE_ERROR_UNABLE_TO_CONNECT_TO_SERVER = 0x01,
	REMOTE_ERROR_SERVER_SEND_INVALID_RESPOSE
} REMOTEERROR;

typedef struct {
	BOOL remoteError;
	LPWSTR function;
	DWORD code;
	REMOTEERROR codeRemote;
} HRSPCLIENTERROR, *PHRSPCLIENTERROR;

typedef struct {
	int placeholder;
} HRSPCLIENTSTRUCT, *PHRSPCLIENTSTRUCT;

BOOL HRSPConnectToServer(const LPCSTR serverAddress, const LPCSTR serverPort, const PHRSPCLIENTSTRUCT client, const PHRSPCLIENTERROR error);
