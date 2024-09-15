#pragma once

#include <hrsp.h>
#include "hrsp_client.h"

typedef struct {
	BOOL running;
	SOCKET socket;
	HRSPDATA data;
	BOOL hasError;
	HRSPCLIENTERROR error;
	HANDLE mutex;
	BYTE flags;
} HRSPCLIENTSTREAMPARAMETER, *PHRSPCLIENTSTREAMPARAMETER;

DWORD WINAPI HRSPClientStreamThread(_In_ PHRSPCLIENTSTREAMPARAMETER parameter);
