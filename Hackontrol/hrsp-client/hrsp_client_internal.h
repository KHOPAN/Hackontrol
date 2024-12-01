#pragma once

#include "hrsp_client.h"

void StreamRequestDevice(const SOCKET socket);

/*#define AM_EXIT                WM_USER
#define AM_QUERY_AUDIO_DEVICE  (WM_USER + 1)
#define AM_QUERY_AUDIO_CAPTURE (WM_USER + 2)

typedef struct {
	HANDLE mutex;
	SOCKET socket;
	HRSPDATA data;
	BOOL hasError;
	HRSPCLIENTERROR error;

	struct {
		BOOL running;
		BYTE flags;
	} stream;
} HRSPCLIENTPARAMETER, *PHRSPCLIENTPARAMETER;

DWORD WINAPI HRSPClientStreamThread(_In_ PHRSPCLIENTPARAMETER parameter);
DWORD WINAPI HRSPClientAudioThread(_In_ PHRSPCLIENTPARAMETER parameter);*/
