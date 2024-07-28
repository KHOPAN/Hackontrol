#include "thread_server.h"
#include "logger.h"

DWORD WINAPI ServerThread(_In_ LPVOID parameter) {
	LOG("[Server Thread]: Initializing Server Thread\n");
	return 0;
}
