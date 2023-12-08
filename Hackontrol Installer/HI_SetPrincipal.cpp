#include <stdio.h>
#include <Windows.h>
#include <taskschd.h>
#include "HackontrolInstaller.h"

void HI_SetPrincipal(ITaskFolder* folder, ITaskDefinition* definition) {
	IPrincipal* principal = NULL;
	printf("Getting Principal Definition\n");
	HRESULT result = definition->get_Principal(&principal);

	if(FAILED(result)) {
		HI_FormatError(result, "ITaskDefinition::get_Principal()");
		principal->Release();
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
		return;
	}

	printf("Putting Run Level: Highest\n");
	result = principal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);

	if(FAILED(result)) {
		HI_FormatError(result, "IPrincipal::put_RunLevel()");
		principal->Release();
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
		return;
	}

	printf("Putting Logon Type: Interactive Token\n");
	result = principal->put_LogonType(TASK_LOGON_INTERACTIVE_TOKEN);
	principal->Release();

	if(FAILED(result)) {
		HI_FormatError(result, "IPrincipal::put_LogonType()");
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
		return;
	}
}
