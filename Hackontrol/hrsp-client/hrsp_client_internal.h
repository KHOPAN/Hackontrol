#pragma once

#include <hrsp.h>

typedef struct {
	SOCKET socket;
	HRSPDATA data;
	BYTE flags;
} HRSPCLIENTSTREAMPARAMETER, *PHRSPCLIENTSTREAMPARAMETER;

DWORD WINAPI HRSPClientStreamThread(_In_ PHRSPCLIENTSTREAMPARAMETER parameter);
