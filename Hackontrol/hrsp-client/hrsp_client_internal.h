#pragma once

#include <hrsp.h>

typedef struct {
	HANDLE mutex;
	BYTE flags;
} HRSPCLIENTSTREAMSENSITIVE;

typedef struct {
	BOOL hasError;
	LPWSTR function;
	DWORD code;
} HRSPCLIENTSTREAMERROR;

typedef struct {
	HRSPCLIENTSTREAMSENSITIVE sensitive;
	HRSPCLIENTSTREAMERROR error;
	SOCKET socket;
	HRSPDATA data;
	BOOL running;
} HRSPCLIENTSTREAMPARAMETER, *PHRSPCLIENTSTREAMPARAMETER;

DWORD WINAPI HRSPClientStreamThread(_In_ PHRSPCLIENTSTREAMPARAMETER parameter);
BOOL HRSPClientEncodeCurrentFrame(const PHRSPCLIENTSTREAMPARAMETER parameter, const int width, const int height, const PBYTE screenshotBuffer, const PBYTE qoiBuffer, const PBYTE previousBuffer);
