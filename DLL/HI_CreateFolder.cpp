#include <stdio.h>
#include <comdef.h>
#include "definition.h"

ITaskFolder* HI_CreateFolder(ITaskService* service, const wchar_t* folderName) {
	ITaskFolder* rootFolder = NULL;
	printf("Getting Root Task Folder\n");
	HRESULT result = service->GetFolder(_bstr_t("\\"), &rootFolder);

	if(FAILED(result)) {
		HU_DisplayError(result, L"ITaskService::GetFolder()");
		service->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
		return NULL;
	}

	ITaskFolder* folder = NULL;
	printf("Creating Task Folder '%ws'\n", folderName);
	result = rootFolder->CreateFolder(_bstr_t(folderName), VARIANT(), &folder);

	if(result == 0x800700B7) {
		printf("Task Folder '%ws' Already Exist\n", folderName);
		printf("Getting Folder Handle\n");
		result = rootFolder->GetFolder(_bstr_t(folderName), &folder);
		rootFolder->Release();

		if(FAILED(result)) {
			HU_DisplayError(result, L"ITaskFolder::GetFolder()");
			service->Release();
			CoUninitialize();
			ExitProcess(static_cast<UINT>(result));
			return NULL;
		}
	} else {
		rootFolder->Release();

		if(FAILED(result)) {
			HU_DisplayError(result, L"ITaskFolder::CreateFolder()");
			service->Release();
			CoUninitialize();
			ExitProcess(static_cast<UINT>(result));
			return NULL;
		}

		printf("Successfully Created Task Folder '%ws'\n", folderName);
	}

	return folder;
}
