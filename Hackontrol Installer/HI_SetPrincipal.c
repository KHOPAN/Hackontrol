#include <stdio.h>
#include <Windows.h>
#include <taskschd.h>
#include "HackontrolInstaller.h"

void HI_SetPrincipal(ITaskFolder* folder, ITaskDefinition* definition) {
	IPrincipal* principal = NULL;
	printf("Getting Principal Definition\n");
	HRESULT result = definition->lpVtbl->get_Principal(definition, &principal);

	if(FAILED(result)) {
		HI_FormatError(result, "ITaskDefinition::get_Principal()");
		principal->lpVtbl->Release(principal);
		definition->lpVtbl->Release(definition);
		folder->lpVtbl->Release(folder);
		CoUninitialize();
		ExitProcess(result);
		return;
	}

	printf("Putting Run Level: Highest\n");
	result = principal->lpVtbl->put_RunLevel(principal, TASK_RUNLEVEL_HIGHEST);

	if(FAILED(result)) {
		HI_FormatError(result, "IPrincipal::put_RunLevel()");
		principal->lpVtbl->Release(principal);
		definition->lpVtbl->Release(definition);
		folder->lpVtbl->Release(folder);
		CoUninitialize();
		ExitProcess(result);
		return;
	}

	printf("Putting Logon Type: Interactive Token\n");
	result = principal->lpVtbl->put_LogonType(principal, TASK_LOGON_INTERACTIVE_TOKEN);
	principal->lpVtbl->Release(principal);

	if(FAILED(result)) {
		HI_FormatError(result, "IPrincipal::put_LogonType()");
		definition->lpVtbl->Release(definition);
		folder->lpVtbl->Release(folder);
		CoUninitialize();
		ExitProcess(result);
	}
}
