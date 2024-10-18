#pragma once

#include "remote.h"

typedef void(__stdcall* TABINITIALIZE)         ();
typedef void(__stdcall* TABUNINITIALIZE)       ();
typedef HWND(__stdcall* TABCLIENTINITIALIZE)   (const HWND parent, const PCLIENT client);
typedef void(__stdcall* TABCLIENTUNINITIALIZE) (const PCLIENT client);

typedef struct {
	LPCWSTR name;
	TABINITIALIZE initialize;
	TABUNINITIALIZE uninitialize;
	TABCLIENTINITIALIZE clientInitialize;
	TABCLIENTUNINITIALIZE clientUninitialize;
	WNDCLASSEXW windowClass;
} TABINITIALIZER, *PTABINITIALIZER;

typedef void(__stdcall* SESSIONTAB) (const PTABINITIALIZER tab);

void __stdcall WindowSessionTabStream(const PTABINITIALIZER tab);
void __stdcall WindowSessionTabAudio(const PTABINITIALIZER tab);
