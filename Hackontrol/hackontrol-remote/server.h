#pragma once

#include <Windows.h>

void ExitRemote(int exitCode);
void RemoteError(DWORD errorCode, const LPWSTR functionName);
void RemoteAddListEntry();
DWORD WINAPI ServerThread(_In_ LPVOID parameter);
