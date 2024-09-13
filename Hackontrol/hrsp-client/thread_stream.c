#include <stdio.h>
#include "hrsp_client_internal.h"

extern BOOL clientHRSPIsRunning;
extern SOCKET clientHRSPSocket;

DWORD WINAPI HRSPClientStreamThread(_In_ LPVOID parameter) {
	while(clientHRSPIsRunning) {

	}

	return 0;
}
