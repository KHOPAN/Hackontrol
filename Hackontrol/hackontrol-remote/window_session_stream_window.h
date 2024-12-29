#pragma once

#include <windowsx.h>
#include <hrsp_remote.h>
#include "window_session.h"

#define CLASS_NAME_POPUP L"HackontrolRemoteSessionTabStreamPopup"

typedef struct {
	HANDLE thread;
	HWND window;
	POINT pressed;
} POPUPDATA, *PPOPUPDATA;

typedef struct {
	LPWSTR name;
	UINT32 identifierLength;
	PBYTE identifier;
	HRSPREMOTESTREAMDEVICETYPE type;
	PPOPUPDATA popup;
} DEVICEENTRY, *PDEVICEENTRY;

DWORD WINAPI popupThread(_In_ PDEVICEENTRY entry);
LRESULT CALLBACK procedurePopup(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam);
