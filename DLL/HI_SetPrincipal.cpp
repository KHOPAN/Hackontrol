#include <stdio.h>
#include <comdef.h>
#include "definition.h"

void HI_SetPrincipal(ITaskFolder* folder, ITaskDefinition* definition) {
	IPrincipal* principal = NULL;
	printf("Getting Principal Definition\n");
	HRESULT result = definition->get_Principal(&principal);

	if(FAILED(result)) {
		HU_DisplayError(result, L"ITaskDefinition::get_Principal()");
		principal->Release();
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(result);
		return;
	}

	printf("Putting Run Level: Highest\n");
	result = principal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);

	if(FAILED(result)) {
		HU_DisplayError(result, L"IPrincipal::put_RunLevel()");
		principal->Release();
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(result);
		return;
	}

	printf("Putting User Id: Administrators\n");
	result = principal->put_UserId(_bstr_t("S-1-5-32-544"));
	principal->Release();

	if(FAILED(result)) {
		HU_DisplayError(result, L"IPrincipal::put_UserId()");
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(result);
	}
}
