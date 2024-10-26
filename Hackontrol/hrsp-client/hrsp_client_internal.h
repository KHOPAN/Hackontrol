#pragma once

#include <hrsp.h>
#include "hrsp_client.h"

#define AM_EXIT               WM_USER
#define AM_QUERY_AUDIO_DEVICE (WM_USER + 1)

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
DWORD WINAPI HRSPClientAudioThread(_In_ PHRSPCLIENTPARAMETER parameter);
