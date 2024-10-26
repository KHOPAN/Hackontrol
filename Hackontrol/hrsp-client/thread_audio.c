#include <stdio.h>
#include <mmdeviceapi.h>
#include "hrsp_client_internal.h"

EXTERN_GUID(CLSID_MMDeviceEnumerator, 0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E);
EXTERN_GUID(IID_IMMDeviceEnumerator, 0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6);

static void queryAudioDevice() {
	IMMDeviceEnumerator* enumerator;
	HRESULT result = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, &enumerator);
	int codeExit = 1;

	if(FAILED(result)) {
		return;
	}

	IMMDeviceCollection* collection;
	result = enumerator->lpVtbl->EnumAudioEndpoints(enumerator, eAll, DEVICE_STATEMASK_ALL, &collection);
	enumerator->lpVtbl->Release(enumerator);

	if(FAILED(result)) {
		return;
	}

	UINT count;
	result = collection->lpVtbl->GetCount(collection, &count);

	if(FAILED(result)) {
		goto releaseCollection;
	}

	printf("Count: %u\n", count);
releaseCollection:
	collection->lpVtbl->Release(collection);
}

DWORD WINAPI HRSPClientAudioThread(_In_ PHRSPCLIENTPARAMETER parameter) {
	if(!parameter) {
		return 1;
	}

	HRESULT result = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	if(FAILED(result)) {
		return 1;
	}

	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		switch(message.message) {
		case AM_EXIT:
			goto uninitialize;
		case AM_QUERY_AUDIO_DEVICE:
			queryAudioDevice();
			break;
		}
	}
uninitialize:
	CoUninitialize();
	return 0;
}
