#pragma once

#include <WinSock2.h>

typedef struct {
	SOCKET socket;
	WCHAR address[16];
	LPWSTR name;
	HANDLE thread;
} CLIENT, *PCLIENT;

DWORD WINAPI ClientThread(_In_ PCLIENT client);
void ClientOpen(PCLIENT client);
void ClientDisconnect(PCLIENT client);
