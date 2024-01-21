#include <stdio.h>
#include "definition.h"

ITaskDefinition* HI_NewTask(ITaskService* service, ITaskFolder* folder) {
	ITaskDefinition* task = NULL;
	printf("Creating New Task\n");
	HRESULT result = service->lpVtbl->NewTask(service, 0, &task);
	service->lpVtbl->Release(service);

	if(FAILED(result)) {
		HU_DisplayError(result, L"ITaskService::NewTask()");
		folder->lpVtbl->Release(folder);
		service->lpVtbl->Release(service);
		CoUninitialize();
		ExitProcess(result);
		return NULL;
	}

	return task;
}
