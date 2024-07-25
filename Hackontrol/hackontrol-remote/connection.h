#pragma once

#include <WS2tcpip.h>

void ExitRemote(int exitCode);
void RemoteError(DWORD errorCode, const LPWSTR functionName);
void RemoteHandleConnection(SOCKET clientSocket);
void RemoteAddListEntry(LPWSTR username);
DWORD WINAPI ServerThread(_In_ LPVOID parameter);
DWORD WINAPI ClientThread(_In_ SOCKET* parameter);
