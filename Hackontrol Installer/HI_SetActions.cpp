#include <stdio.h>
#include <Windows.h>
#include <comdef.h>
#include <taskschd.h>
#include "HackontrolInstaller.h"

void HI_SetActions(ITaskFolder* folder, ITaskDefinition* definition, const wchar_t* path) {
	IActionCollection* collection = NULL;
	printf("Getting Action Definition\n");
	HRESULT result = definition->get_Actions(&collection);

	if(FAILED(result)) {
		HI_FormatError(result, "ITaskDefinition::get_Actions()");
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
		return;
	}

	IAction* action = NULL;
	printf("Creating Execute Action\n");
	result = collection->Create(TASK_ACTION_EXEC, &action);
	collection->Release();

	if(FAILED(result)) {
		HI_FormatError(result, "IActionCollection::Create()");
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
		return;
	}

	IExecAction* executeAction = NULL;
	printf("Casting To Execute Action\n");
	result = action->QueryInterface(IID_IExecAction, reinterpret_cast<void**>(&executeAction));
	action->Release();

	if(FAILED(result)) {
		HI_FormatError(result, "IAction::QueryInterface()");
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
		return;
	}

	printf("Putting Executable Path\n");
	result = executeAction->put_Path(_bstr_t(path));
	executeAction->Release();

	if(FAILED(result)) {
		HI_FormatError(result, "IExecAction::put_Path()");
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
	}
}
