#include "remote.h"

DWORD WINAPI WindowSession(_In_ LPVOID parameter) {
	LOG("[Session]: Initializing\n");
	DWORD codeExit = 1;
	codeExit = 0;
functionExit:
	LOG("[Session]: Exit with code: %d\n", codeExit);
	return codeExit;
}
