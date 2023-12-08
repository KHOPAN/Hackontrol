#include <stdio.h>
#include <Windows.h>
#include <comdef.h>
#include <taskschd.h>
#include "HackontrolInstaller.h"

void HI_SetSettings(ITaskFolder* folder, ITaskDefinition* definition) {
	ITaskSettings* settings = NULL;
	printf("Getting Settings Definition\n");
	HRESULT result = definition->get_Settings(&settings);

	if(FAILED(result)) {
		HI_FormatError(result, "ITaskDefinition::get_Settings()");
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
		return;
	}

	printf("Putting Allow Demand Start: True\n");
	result = settings->put_AllowDemandStart(VARIANT_TRUE);

	if(FAILED(result)) {
		HI_FormatError(result, "ITaskSettings::put_AllowDemandStart()");
		settings->Release();
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
		return;
	}

	printf("Putting Restart Interval: 1m\n");
	result = settings->put_RestartInterval(_bstr_t(L"PT1M"));

	if(FAILED(result)) {
		HI_FormatError(result, "ITaskSettings::put_RestartInterval()");
		settings->Release();
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
		return;
	}

	printf("Putting Restart Count: 5\n");
	result = settings->put_RestartCount(5);

	if(FAILED(result)) {
		HI_FormatError(result, "ITaskSettings::put_RestartCount()");
		settings->Release();
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
		return;
	}
	
	printf("Putting Execution Time Limit: Infinite\n");
	result = settings->put_ExecutionTimeLimit(_bstr_t(L"PT0M"));

	if(FAILED(result)) {
		HI_FormatError(result, "ITaskSettings::put_ExecutionTimeLimit()");
		settings->Release();
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
	}

	printf("Putting Disallow Start If On Batteries: False\n");
	result = settings->put_DisallowStartIfOnBatteries(VARIANT_FALSE);

	if(FAILED(result)) {
		HI_FormatError(result, "ITaskSettings::put_DisallowStartIfOnBatteries()");
		settings->Release();
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
	}

	printf("Putting Stop If Going On Batteries: False\n");
	result = settings->put_StopIfGoingOnBatteries(VARIANT_FALSE);

	if(FAILED(result)) {
		HI_FormatError(result, "ITaskSettings::put_StopIfGoingOnBatteries()");
		settings->Release();
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
	}

	printf("Putting Run Only If Network Available: True\n");
	result = settings->put_RunOnlyIfNetworkAvailable(VARIANT_TRUE);

	if(FAILED(result)) {
		HI_FormatError(result, "ITaskSettings::put_RunOnlyIfNetworkAvailable()");
		settings->Release();
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
	}


	printf("Putting Hidden: True\n");
	result = settings->put_Hidden(VARIANT_TRUE);
	settings->Release();

	if(FAILED(result)) {
		HI_FormatError(result, "ITaskSettings::put_Hidden()");
		definition->Release();
		folder->Release();
		CoUninitialize();
		ExitProcess(static_cast<UINT>(result));
	}
}
