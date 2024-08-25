#pragma once

#include <Windows.h>

typedef struct {
	BOOL present;
	DWORD code;
	LPWSTR function;
} HRSPCLIENTERROR;

typedef struct {
	HRSPCLIENTERROR error;
} HRSPCLIENTSTRUCT, *PHRSPCLIENTSTRUCT;

void HRSPConnectToServer(const LPCSTR serverAddress, const LPCSTR serverPort, const PHRSPCLIENTSTRUCT client);
