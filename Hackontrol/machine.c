#include <Windows.h>
#include <stdio.h>
#include "error.h"
#include "machine.h"

char* machine_getGUID() {
	LSTATUS status = 0;
	DWORD size = 0;

	if(status = RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography", "MachineGuid", RRF_RT_REG_SZ, NULL, NULL, &size) != ERROR_SUCCESS) {
		error_showError(status, L"RegGetValueA");
		ExitProcess(1);
		return NULL;
	}

	if(size <= 0) {
		MessageBoxW(NULL, L"Error: Zero-length string", L"Error", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
		ExitProcess(1);
		return NULL;
	}

	void* buffer = malloc(size * sizeof(char));

	if(!buffer) {
		error_showError(ERROR_OUTOFMEMORY, L"malloc");
		ExitProcess(1);
		return NULL;
	}

	if(status = RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography", "MachineGuid", RRF_RT_REG_SZ, NULL, buffer, &size) != ERROR_SUCCESS) {
		error_showError(status, L"RegGetValueA");
		ExitProcess(1);
		return NULL;
	}

	return buffer;
}
