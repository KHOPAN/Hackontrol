#include <stdio.h>
#include <Windows.h>
#include <comdef.h>
#include <taskschd.h>
#include "HackontrolInstaller.h"

void HI_RegisterTask(ITaskFolder* folder, ITaskDefinition* definition, const wchar_t* taskName) {
	printf("Deleting Already Exist Task, If Any\n");
	folder->DeleteTask(_bstr_t(taskName), NULL);
	IRegisteredTask* task = NULL;
	printf("Registering Task Definition\n");
	printf("User Account: Administrators\n");
	HRESULT result = folder->RegisterTaskDefinition(_bstr_t(taskName), definition, TASK_CREATE_OR_UPDATE, VARIANT(), VARIANT(), TASK_LOGON_GROUP, _variant_t(""), &task);
	
	if(FAILED(result)) {
		HI_FormatError(result, "ITaskFolder::RegisterTaskDefinition()");
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
		return;
	}

	printf("Task Registered Successfully\n");
	MessageBoxW(NULL, L"Hackontrol successfully installed on your machine", L"Hackontrol Installer", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	IRunningTask* runningTask = NULL;
	result = task->Run(VARIANT(), &runningTask);
	task->Release();

	if(FAILED(result)) {
		HI_FormatError(result, "IRegisteredTask::Run()");
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
	}

	runningTask->Release();
}
