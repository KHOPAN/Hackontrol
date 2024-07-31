#include "thread_window.h"
#include "logger.h"

DWORD WINAPI WindowThread(_In_ LPVOID parameter) {
	LOG("[Window Thread]: Hello from window thread\n");
	return 0;
}
