#include "remote.h"

DWORD WINAPI ThreadClient(_In_ PCLIENT client) {
	if(!client) {
		LOG("[Client]: Empty thread parameter\n");
		return 1;
	}

	DWORD codeExit = 1;
	LOG("[Client %ws]: Initializing\n", client->address);
	codeExit = 0;
functionExit:
	LOG("[Client %ws]: Exit with code: %d\n", client->address, codeExit);
	CloseHandle(client->thread);
	LocalFree(client);
	return codeExit;
}
