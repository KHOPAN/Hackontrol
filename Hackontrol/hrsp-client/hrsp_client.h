#pragma once

#include <Windows.h>

typedef struct {

} HRSPCLIENTSTRUCT, *PHRSPCLIENTSTRUCT;

BOOL HRSPConnectToServer(const LPCSTR serverAddress, const LPCSTR serverPort, const PHRSPCLIENTSTRUCT client);
