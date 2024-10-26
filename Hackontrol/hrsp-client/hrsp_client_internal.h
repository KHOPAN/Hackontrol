#pragma once

#include <hrsp.h>
#include "hrsp_client.h"

typedef struct {
	HANDLE mutex;
	SOCKET socket;
	HRSPDATA data;
	BOOL running;
	BOOL hasError;
	HRSPCLIENTERROR error;

	struct {
		BYTE flags;
	} stream;
} HRSPCLIENTPARAMETER, *PHRSPCLIENTPARAMETER;

DWORD WINAPI HRSPClientStreamThread(_In_ PHRSPCLIENTPARAMETER parameter);
DWORD WINAPI HRSPClientAudioThread(_In_ PHRSPCLIENTPARAMETER parameter);
