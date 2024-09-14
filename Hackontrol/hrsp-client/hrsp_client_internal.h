#pragma once

#include <hrsp.h>

typedef struct {
	SOCKET socket;
	HRSPDATA data;
	BYTE flags;
	BOOL running;
} HRSPCLIENTSTREAMPARAMETER, *PHRSPCLIENTSTREAMPARAMETER;

DWORD WINAPI HRSPClientStreamThread(_In_ PHRSPCLIENTSTREAMPARAMETER parameter);
BOOL HRSPClientEncodeCurrentFrame(const int width, const int height, const PBYTE screenshotBuffer, const PBYTE qoiBuffer, const PBYTE previousBuffer);
