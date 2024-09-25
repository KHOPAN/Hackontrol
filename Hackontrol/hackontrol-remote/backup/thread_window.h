#pragma once

#include "thread_client.h"

BOOL ClientWindowInitialize(const HINSTANCE instance);
DWORD WINAPI ClientWindowThread(_In_ PCLIENT client);
void ClientWindowExit(const PCLIENT client);
