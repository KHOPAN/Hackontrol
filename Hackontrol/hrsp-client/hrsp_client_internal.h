#pragma once

#include <Windows.h>

typedef struct {
	BYTE placeholder;
} HRSPCLIENTSTREAMPARAMETER, *PHRSPCLIENTSTREAMPARAMETER;

DWORD WINAPI HRSPClientStreamThread(_In_ PHRSPCLIENTSTREAMPARAMETER parameter);
