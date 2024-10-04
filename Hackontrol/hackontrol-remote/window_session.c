#include "remote.h"

DWORD WINAPI WindowSession(_In_ PCLIENT client) {
	if(!client) {
		LOG("[Session]: Empty thread parameter\n");
		return 1;
	}

	LOG("[Session %ws]: Initializing\n", client->address);
	DWORD codeExit = 1;
	codeExit = 0;
functionExit:
	LOG("[Session %ws]: Exit with code: %d\n", client->address, codeExit);
	CloseHandle(client->session.thread);
	client->session.thread = NULL;
	return codeExit;
}

void WindowSessionClose(const PCLIENT client) {

}
