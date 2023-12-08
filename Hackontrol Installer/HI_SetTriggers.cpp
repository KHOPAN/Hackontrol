#include <stdio.h>
#include <Windows.h>
#include <comdef.h>
#include <taskschd.h>
#include "HackontrolInstaller.h"

void HI_SetTriggers(ITaskFolder* folder, ITaskDefinition* definition) {
	ITriggerCollection* collection = NULL;
	printf("Getting Trigger Definition\n");
	HRESULT result = definition->get_Triggers(&collection);

	if(FAILED(result)) {
		HI_FormatError(result, "ITaskDefinition::get_Triggers()");
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
		return;
	}

	ITrigger* trigger = NULL;
	printf("Creating Logon Trigger\n");
	result = collection->Create(TASK_TRIGGER_LOGON, &trigger);
	collection->Release();

	if(FAILED(result)) {
		HI_FormatError(result, "ITriggerCollection::Create()");
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
		return;
	}

	ILogonTrigger* logonTrigger = NULL;
	printf("Casting To Logon Trigger\n");
	result = trigger->QueryInterface(IID_ILogonTrigger, reinterpret_cast<void**>(&logonTrigger));
	trigger->Release();

	if(FAILED(result)) {
		HI_FormatError(result, "ITrigger::QueryInterface()");
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
		return;
	}

	printf("Putting Logon Trigger ID: Trigger1\n");
	result = logonTrigger->put_Id(_bstr_t(L"Trigger1"));
	logonTrigger->Release();

	if(FAILED(result)) {
		HI_FormatError(result, "ILogonTrigger::put_Id()");
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
	}
}
