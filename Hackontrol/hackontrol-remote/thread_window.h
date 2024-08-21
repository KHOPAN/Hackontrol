#pragma once

#include "thread_client.h"

#define CLASS_CLIENT_WINDOW L"HackontrolRemoteClientWindow"

BOOL WindowRegisterClass(HINSTANCE instance);
DWORD WINAPI WindowThread(_In_ PCLIENT client);
void ExitClientWindow(const PCLIENT client);
