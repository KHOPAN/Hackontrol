#pragma once

#include <Windows.h>

void ExitHackontrolRemote(int exitCode);
void HackontrolRemoteError(DWORD errorCode, const LPWSTR functionName);
DWORD WINAPI ServerThread(_In_ LPVOID parameter);
