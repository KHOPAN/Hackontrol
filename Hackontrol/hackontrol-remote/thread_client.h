#pragma once

#include <WinSock2.h>

typedef enum {
	SEND_METHOD_FULL         = 1,
	SEND_METHOD_BOUNDARY     = 2,
	SEND_METHOD_COLOR        = 3,
	SEND_METHOD_UNCOMPRESSED = 4
} SENDMETHOD;

typedef struct {
	BOOL active;
	SOCKET socket;
	WCHAR address[16];
	LPWSTR name;
	HANDLE thread;
	HANDLE windowThread;
	HWND clientWindow;
	BOOL streaming;
	SENDMETHOD sendMethod;
} CLIENT, *PCLIENT;

DWORD WINAPI ClientThread(_In_ PCLIENT client);
void ClientOpen(PCLIENT client);
void ClientDisconnect(PCLIENT client);
