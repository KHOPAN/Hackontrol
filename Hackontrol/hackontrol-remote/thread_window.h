#pragma once

#include "thread_client.h"

#define CLASS_CLIENT_WINDOW L"HackontrolRemoteClientWindow"

BOOL WindowRegisterClass();
DWORD WINAPI WindowThread(_In_ PCLIENT client);
