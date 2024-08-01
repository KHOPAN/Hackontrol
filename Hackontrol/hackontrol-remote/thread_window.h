#pragma once

#include <Windows.h>

#define CLASS_CLIENT_WINDOW L"HackontrolRemoteClientWindow"

BOOL WindowRegisterClass();
DWORD WINAPI WindowThread(_In_ LPVOID parameter);
