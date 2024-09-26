#include "remote.h"

DWORD WINAPI ThreadServer(_In_ LPVOID parameter) {
	LOG("[Server]: Initializing\n");
	DWORD codeExit = 1;
	codeExit = 0;
functionExit:
	LOG("[Server]: Exit with code: %d\n", codeExit);
	return codeExit;
}
