#include "remote.h"

DWORD WINAPI ThreadClient(_In_ PCLIENT client) {
	DWORD codeExit = 1;

	if(!client) {
		LOG("[Client]: Empty thread parameter\n");
		goto functionExit;
	}

	LOG("[Client %ws]: Initializing\n", client->address);
	codeExit = 0;
functionExit:
	LOG("[Client %ws]: Exit with code: %d\n", client->address, codeExit);
	return codeExit;
}
