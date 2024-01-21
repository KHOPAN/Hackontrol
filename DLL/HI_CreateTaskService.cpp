#include <stdio.h>
#include "definition.h"

ITaskService* HI_CreateTaskService() {
	ITaskService* service = NULL;
	printf("Creating Service Instance\n");
	HRESULT result = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**) &service);

	if(FAILED(result)) {
		HU_DisplayError(result, L"CoCreateInstance()");
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
		return NULL;
	}

	printf("Connecting to Task Service\n");
	result = service->Connect(VARIANT(), VARIANT(), VARIANT(), VARIANT());

	if(FAILED(result)) {
		HU_DisplayError(result, L"ITaskService::Connect()");
		service->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
		return NULL;
	}

	printf("Connected to Task Service\n");
	return service;
}
