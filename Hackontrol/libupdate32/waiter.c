#include "update.h"

void WaitForLibdll32ToExit(DWORD processIdentifier) {
	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processIdentifier);

	if(!process) {
		return;
	}

	WaitForSingleObject(process, INFINITE);
	CloseHandle(process);
}
