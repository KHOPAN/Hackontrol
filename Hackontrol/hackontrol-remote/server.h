#pragma once

#include <WS2tcpip.h>

void ExitRemote(int exitCode);
void RemoteError(DWORD errorCode, const LPWSTR functionName);
void RemoteHandleConnection(const SOCKET clientSocket);
void RemoteAddListEntry();
DWORD WINAPI ServerThread(_In_ LPVOID parameter);
