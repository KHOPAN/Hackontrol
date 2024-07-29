#pragma once

#include <WinSock2.h>

typedef struct {
	SOCKET socket;
	WCHAR address[16];
} CLIENT, *PCLIENT;
