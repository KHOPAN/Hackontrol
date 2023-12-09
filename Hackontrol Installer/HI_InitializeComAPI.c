#include <stdio.h>
#include <Windows.h>
#include "HackontrolInstaller.h"

void HI_InitializeComAPI() {
	printf("Initializing Com API\n");
	HRESULT result = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	if(FAILED(result)) {
		HI_FormatError(result, "CoInitializeEx()");
		ExitProcess(result);
		return;
	}

	printf("Initializing Com API Security\n");
	result = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0, NULL);
	
	if(FAILED(result)) {
		HI_FormatError(result, "CoInitializeSecurity()");
		CoUninitialize();
		ExitProcess(result);
	}
}
