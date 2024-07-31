#include "window_client.h"
#include "logger.h"

DWORD WINAPI ClientWindowThread(_In_ LPVOID parameter) {
	LOG("[Window Thread]: Hello from client window thread\n");
	return 0;
}
