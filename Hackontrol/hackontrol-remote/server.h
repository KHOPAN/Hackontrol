#pragma once

#include <Windows.h>

void ExitHackontrolRemote(int exitCode);
void HackontrolRemoteError(DWORD errorCode, const LPWSTR functionName);
void HackontrolAddListEntry();
DWORD WINAPI ServerThread(_In_ LPVOID parameter);