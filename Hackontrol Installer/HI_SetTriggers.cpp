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

	trigger->Release();
}
