#pragma once

#include "remote.h"

#define REMOTE_CLASS_TAB_STREAM L"HackontrolRemoteSessionTabStream"

typedef HWND(__stdcall* TABFUNCTION) (const HWND parent, const PCLIENT client);

typedef struct {
	LPWSTR name;
	LPWSTR className;
	TABFUNCTION function;
	WNDPROC procedure;
} SESSIONTAB;

HWND __stdcall WindowSessionTabStream(const HWND parent, const PCLIENT client);
LRESULT CALLBACK WindowSessionTabStreamProcedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam);
HWND __stdcall WindowSessionTabAudio();
