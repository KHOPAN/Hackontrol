#include <stdio.h>
#include "hrsp_client_internal.h"

DWORD WINAPI HRSPClientStreamThread(_In_ LPVOID parameter) {
	printf("Hello from stream thread\n");
	return 0;
}
