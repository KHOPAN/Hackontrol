#pragma once

#include <WS2tcpip.h>

typedef struct {
	SOCKET clientSocket;
	LPWSTR address;
} CLIENTPARAMETER;

void ExitRemote(int exitCode);
void RemoteError(DWORD errorCode, const LPWSTR functionName);
void RemoteHandleConnection(SOCKET clientSocket, LPWSTR address);
void RemoteAddListEntry(LPWSTR username, LPWSTR address);
DWORD WINAPI ServerThread(_In_ LPVOID parameter);
DWORD WINAPI ClientThread(_In_ CLIENTPARAMETER* parameter);
