#include <stdio.h>
#include "thread_server.h"

DWORD WINAPI ServerThread(_In_ LPVOID parameter) {
	printf("Hello from server thread\n");
	return 0;
}
