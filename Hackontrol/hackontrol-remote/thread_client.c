#include "thread_client.h"
#include "logger.h"

DWORD WINAPI ClientThread(_In_ PCLIENT client) {
	if(!client) {
		return 1;
	}

	LOG("[Client Thread %ws]: Hello from client thread\n" COMMA client->address);
	closesocket(client->socket);
	CloseHandle(client->thread);
	LOG("[Client Thread %ws]: Exiting the client thread (Exit code: %d)\n" COMMA client->address COMMA 0);
	LocalFree(client);
	return 0;
}
