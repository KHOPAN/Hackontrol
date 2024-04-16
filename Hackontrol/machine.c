#include <Windows.h>
#include <stdio.h>
#include "error.h"
#include "machine.h"

void machine_getUUID() {
	LSTATUS status = 0;
	DWORD size = 0;

	if(status = RegGetValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography", L"MachineGuid", RRF_RT_REG_SZ, NULL, NULL, &size) != ERROR_SUCCESS) {
		error_showError(status, L"RegGetValueW");
		ExitProcess(1);
		return;
	}

	if(size <= 0) {
		MessageBoxW(NULL, L"Error: Zero-length string", L"Error", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
		ExitProcess(1);
		return;
	}

	HANDLE heap = GetProcessHeap();

	if(!heap) {
		error_showError(GetLastError(), L"GetProcessHeap");
		ExitProcess(1);
		return;
	}

	LPVOID buffer = HeapAlloc(heap, HEAP_ZERO_MEMORY, size * sizeof(wchar_t));

	if(!buffer) {
		error_showError(ERROR_NOT_ENOUGH_MEMORY, L"HeapAlloc");
		ExitProcess(1);
		return;
	}

	if(status = RegGetValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography", L"MachineGuid", RRF_RT_REG_SZ, NULL, buffer, &size) != ERROR_SUCCESS) {
		error_showError(status, L"RegGetValueW");
		ExitProcess(1);
		return;
	}

	printf("Size: %d\nText: %ws\n", size, (wchar_t*) buffer);

	if(!HeapFree(heap, 0, buffer)) {
		error_showError(GetLastError(), L"HeapFree");
		ExitProcess(1);
	}
}
