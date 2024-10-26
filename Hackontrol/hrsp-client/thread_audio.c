#include <stdio.h>
#include <mmdeviceapi.h>
#include "hrsp_client_internal.h"
#include <libkhopanlist.h>
#include <hrsp_packet.h>
#include <hrsp_remote.h>

EXTERN_GUID(CLSID_MMDeviceEnumerator, 0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E);
EXTERN_GUID(IID_IMMDeviceEnumerator, 0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6);

static void queryAudioDevice(const PHRSPCLIENTPARAMETER parameter) {
	DATASTREAM stream;

	if(!KHOPANStreamInitialize(&stream, 64)) {
		return;
	}

	IMMDeviceEnumerator* enumerator;
	HRESULT result = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, &enumerator);
	int codeExit = 1;

	if(FAILED(result)) {
		goto freeStream;
	}

	IMMDeviceCollection* collection;
	result = enumerator->lpVtbl->EnumAudioEndpoints(enumerator, eAll, DEVICE_STATEMASK_ALL, &collection);
	enumerator->lpVtbl->Release(enumerator);

	if(FAILED(result)) {
		goto freeStream;
	}

	UINT count;
	result = collection->lpVtbl->GetCount(collection, &count);

	if(FAILED(result)) {
		goto releaseCollection;
	}

	printf("Count: %u\n", count);
	BYTE buffer[4];
	buffer[0] = (count >> 24) & 0xFF;
	buffer[1] = (count >> 16) & 0xFF;
	buffer[2] = (count >> 8) & 0xFF;
	buffer[3] = count & 0xFF;
	KHOPANStreamAdd(&stream, buffer, 4);
	HRSPPACKET packet = {0};
	packet.size = (int) stream.size;
	packet.type = HRSP_REMOTE_CLIENT_AUDIO_DEVICE_RESULT;
	packet.data = stream.data;
	HRSPSendPacket(parameter->socket, &parameter->data, &packet, NULL);
releaseCollection:
	collection->lpVtbl->Release(collection);
freeStream:
	KHOPANStreamFree(&stream);
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
			queryAudioDevice(parameter);
			break;
		}
	}
uninitialize:
	CoUninitialize();
	return 0;
}
