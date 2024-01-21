#include <stdio.h>
#include "definition.h"

void HI_InitializeComAPI() {
	printf("Initializing Com API\n");
	HRESULT result = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	if(FAILED(result)) {
		HU_DisplayError(result, L"CoInitializeEx()");
		ExitProcess(result);
		return;
	}

	printf("Initializing Com API Security\n");
	result = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0, NULL);

	if(FAILED(result)) {
		HU_DisplayError(result, L"CoInitializeSecurity()");
		CoUninitialize();
		ExitProcess(result);
	}
}
