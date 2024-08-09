#pragma once

#include <WinSock2.h>

typedef struct {
	BOOL active;
	SOCKET socket;
	WCHAR address[16];
	LPWSTR name;
	HANDLE thread;
	HANDLE windowThread;
	HWND clientWindow;
	BOOL streaming;
} CLIENT, *PCLIENT;

DWORD WINAPI ClientThread(_In_ PCLIENT client);
void ClientOpen(PCLIENT client);
void ClientDisconnect(PCLIENT client);
