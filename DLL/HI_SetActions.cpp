#include <stdio.h>
#include <comdef.h>
#include "definition.h"

void HI_SetActions(ITaskFolder* folder, ITaskDefinition* definition, const wchar_t* path, const wchar_t* argument) {
	IActionCollection* collection = NULL;
	printf("Getting Action Definition\n");
	HRESULT result = definition->get_Actions(&collection);

	if(FAILED(result)) {
		HU_DisplayError(result, L"ITaskDefinition::get_Actions()");
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
		HU_DisplayError(result, L"IActionCollection::Create()");
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
		HU_DisplayError(result, L"IAction::QueryInterface()");
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
		return;
	}

	printf("Putting DLL Path\n");
	result = executeAction->put_Path(_bstr_t(path));

	if(FAILED(result)) {
		HU_DisplayError(result, L"IExecAction::put_Path()");
		executeAction->Release();
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
		return;
	}

	printf("Putting DLL Argument\n");
	result = executeAction->put_Arguments(_bstr_t(argument));
	executeAction->Release();

	if(FAILED(result)) {
		HU_DisplayError(result, L"IExecAction::put_Arguments()");
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
	}
}
