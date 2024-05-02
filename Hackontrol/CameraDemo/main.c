#include <stdio.h>
#include <Windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <khopanerror.h>

int main(int argc, char** argv) {
	HRESULT result = CoInitialize(NULL);

	if(FAILED(result)) {
		KHWin32ConsoleErrorW(result, L"CoInitialize");
		return 1;
	}

	result = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);

	if(FAILED(result)) {
		KHWin32ConsoleErrorW(result, L"MFStartup");
		return 1;
	}

	IMFAttributes* attributes = NULL;
	result = MFCreateAttributes(&attributes, 0);

	if(FAILED(result)) {
		KHWin32ConsoleErrorW(result, L"MFCreateAttributes");
		return 1;
	}

	result = attributes->lpVtbl->SetGUID(attributes, &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
	
	if(FAILED(result)) {
		KHWin32ConsoleErrorW(result, L"IMFAttributes::SetGUID");
		return 1;
	}

	IMFActivate** activates = NULL;
	unsigned long count = 0;
	result = MFEnumDeviceSources(attributes, &activates, &count);

	if(FAILED(result)) {
		KHWin32ConsoleErrorW(result, L"IMFAttributes::SetGUID");
		return 1;
	}

	if(count == 0) {
		printf("Error: No devices were found\n");
		return 1;
	}

	IMFActivate* activate = activates[0];
	IMFMediaSource* source = NULL;
	result = activate->lpVtbl->ActivateObject(activate, &IID_IMFMediaSource, &source);

	if(FAILED(result)) {
		KHWin32ConsoleErrorW(result, L"IMFActivate::ActivateObject");
		return 1;
	}

	source->lpVtbl->AddRef(source);
	return 0;
}
