#include "remote.h"

DWORD WINAPI ThreadClient(_In_ LPVOID parameter) {
	DWORD codeExit = 1;
	LOG("[Client]: Initializing\n");
	codeExit = 0;
functionExit:
	LOG("[Client]: Exit with code: %d\n", codeExit);
	return codeExit;
}
