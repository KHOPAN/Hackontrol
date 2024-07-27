#pragma once

#include <WS2tcpip.h>

typedef struct {
	SOCKET clientSocket;
	LPWSTR username;
	LPWSTR address;
	HANDLE clientThread;
} CLIENTENTRY;

void ExitRemote(int exitCode);
void RemoteError(DWORD errorCode, const LPWSTR functionName);
void RemoteHandleConnection(const SOCKET clientSocket, LPWSTR address);
void RemoteRefreshClientList();
void RemoteRemoveEntry(const SOCKET clientSocket);
DWORD WINAPI ServerThread(_In_ LPVOID parameter);
DWORD WINAPI ClientThread(_In_ CLIENTENTRY* parameter);
