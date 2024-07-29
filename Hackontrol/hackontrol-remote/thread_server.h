#pragma once

#include <Windows.h>

DWORD WINAPI ServerThread(_In_ LPVOID parameter);
void ExitServerThread();
