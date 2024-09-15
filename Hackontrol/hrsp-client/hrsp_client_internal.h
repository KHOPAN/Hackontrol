#pragma once

#include <hrsp.h>
#include "hrsp_client.h"

typedef struct {
	HANDLE mutex;
	BYTE flags;
} HRSPCLIENTSTREAMSENSITIVE;

typedef struct {
	HRSPCLIENTSTREAMSENSITIVE sensitive;
	BOOL hasError;
	HRSPCLIENTERROR error;
	SOCKET socket;
	HRSPDATA data;
	BOOL running;
} HRSPCLIENTSTREAMPARAMETER, *PHRSPCLIENTSTREAMPARAMETER;

DWORD WINAPI HRSPClientStreamThread(_In_ PHRSPCLIENTSTREAMPARAMETER parameter);
