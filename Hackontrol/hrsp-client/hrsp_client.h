#pragma once

#include <Windows.h>

typedef struct {
	int placeholder;
} HRSPCLIENTSTRUCT, *PHRSPCLIENTSTRUCT;

BOOL HRSPConnectToServer(const LPCSTR serverAddress, const LPCSTR serverPort, const PHRSPCLIENTSTRUCT client);
