#pragma once

#include "remote.h"

typedef void(__stdcall* TABINITIALIZE)         (const PULONGLONG data);
typedef void(__stdcall* TABUNINITIALIZE)       (const PULONGLONG data);
typedef HWND(__stdcall* TABCLIENTINITIALIZE)   (const PCLIENT client, const PULONGLONG data, const HWND parent);
typedef void(__stdcall* TABCLIENTUNINITIALIZE) (const PCLIENT client, const PULONGLONG data);
typedef BOOLEAN(__stdcall* TABPACKETHANDLER)   (const PCLIENT client, const PULONGLONG data, const PHRSPPACKET packet);

typedef struct {
	LPCWSTR name;
	TABINITIALIZE initialize;
	TABUNINITIALIZE uninitialize;
	TABCLIENTINITIALIZE clientInitialize;
	TABCLIENTUNINITIALIZE clientUninitialize;
	TABPACKETHANDLER packetHandler;
	BOOLEAN alwaysProcessPacket;
	WNDCLASSEXW windowClass;
	ULONGLONG data;
} TABINITIALIZER, *PTABINITIALIZER;

void __stdcall WindowSessionTabStream(const PTABINITIALIZER tab);
void __stdcall WindowSessionTabAudio(const PTABINITIALIZER tab);
