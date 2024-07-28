#include "thread_server.h"
#include "logger.h"

DWORD WINAPI ServerThread(_In_ LPVOID parameter) {
	LOG("[Server Thread]: Hello from server thread\n");
	return 0;
}
