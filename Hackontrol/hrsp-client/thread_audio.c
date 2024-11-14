/*#include <stdio.h>
#include <libkhopan.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include "hrsp_client_internal.h"
#include <libkhopanlist.h>
#include <hrsp_packet.h>
#include <hrsp_remote.h>
#include <functiondiscoverykeys_devpkey.h>

#define AUDIO_DEVICE_ACTIVE     0x01
#define AUDIO_DEVICE_DISABLED   0x02
#define AUDIO_DEVICE_NOTPRESENT 0x03
#define AUDIO_DEVICE_UNPLUGGED  0x04

EXTERN_GUID(CLSID_MMDeviceEnumerator, 0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E);
EXTERN_GUID(IID_IMMDeviceEnumerator,  0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6);
EXTERN_GUID(IID_IAudioClient,         0x1CB9AD4C, 0xDBFA, 0x4C32, 0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2);
EXTERN_GUID(IID_IAudioCaptureClient,  0xC8ADBD64, 0xE71E, 0x48A0, 0xA4, 0xDE, 0x18, 0x5C, 0x39, 0x5C, 0xD3, 0x17);

static void queryAudioDevice(const PHRSPCLIENTPARAMETER parameter) {
	DATASTREAM stream;

	if(!KHOPANStreamInitialize(&stream, 256)) {
		return;
	}

	IMMDeviceEnumerator* enumerator;
	HRESULT result = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, &enumerator);

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

	BYTE buffer[4];
	buffer[0] = (count >> 24) & 0xFF;
	buffer[1] = (count >> 16) & 0xFF;
	buffer[2] = (count >> 8) & 0xFF;
	buffer[3] = count & 0xFF;

	if(!KHOPANStreamAdd(&stream, buffer, 4)) {
		goto releaseCollection;
	}

	for(UINT i = 0; i < count; i++) {
		IMMDevice* device;
		result = collection->lpVtbl->Item(collection, i, &device);

		if(FAILED(result)) {
			goto releaseCollection;
		}

		DWORD state;
		result = device->lpVtbl->GetState(device, &state);

		if(FAILED(result)) {
			device->lpVtbl->Release(device);
			goto releaseCollection;
		}

		switch(state) {
		case DEVICE_STATE_ACTIVE:
			buffer[0] = AUDIO_DEVICE_ACTIVE;
			break;
		case DEVICE_STATE_DISABLED:
			buffer[0] = AUDIO_DEVICE_DISABLED;
			break;
		case DEVICE_STATE_NOTPRESENT:
			buffer[0] = AUDIO_DEVICE_NOTPRESENT;
			break;
		case DEVICE_STATE_UNPLUGGED:
			buffer[0] = AUDIO_DEVICE_UNPLUGGED;
			break;
		}

		if(!KHOPANStreamAdd(&stream, buffer, 1)) {
			device->lpVtbl->Release(device);
			goto releaseCollection;
		}

		IPropertyStore* store;
		result = device->lpVtbl->OpenPropertyStore(device, STGM_READ, &store);

		if(FAILED(result)) {
			device->lpVtbl->Release(device);
			goto releaseCollection;
		}

		PROPVARIANT variant = {0};
		result = store->lpVtbl->GetValue(store, &PKEY_DeviceInterface_FriendlyName, &variant);
		store->lpVtbl->Release(store);

		if(FAILED(result)) {
			device->lpVtbl->Release(device);
			goto releaseCollection;
		}

		UINT size = (UINT) (wcslen(variant.pwszVal) * sizeof(WCHAR));
		buffer[0] = (size >> 24) & 0xFF;
		buffer[1] = (size >> 16) & 0xFF;
		buffer[2] = (size >> 8) & 0xFF;
		buffer[3] = size & 0xFF;

		if(!KHOPANStreamAdd(&stream, buffer, 4) || !KHOPANStreamAdd(&stream, (PBYTE) variant.pwszVal, size)) {
			device->lpVtbl->Release(device);
			goto releaseCollection;
		}

		LPWSTR identifier;
		result = device->lpVtbl->GetId(device, &identifier);
		device->lpVtbl->Release(device);

		if(FAILED(result)) {
			goto releaseCollection;
		}

		size = (UINT) (wcslen(identifier) * sizeof(WCHAR));
		buffer[0] = (size >> 24) & 0xFF;
		buffer[1] = (size >> 16) & 0xFF;
		buffer[2] = (size >> 8) & 0xFF;
		buffer[3] = size & 0xFF;

		if(!KHOPANStreamAdd(&stream, buffer, 4) || !KHOPANStreamAdd(&stream, (PBYTE) identifier, size)) {
			CoTaskMemFree(identifier);
			goto releaseCollection;
		}

		CoTaskMemFree(identifier);
	}

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

static void captureAudio(const LPCWSTR identifier) {
	printf("Identifier: %ws\n", identifier);
	IMMDeviceEnumerator* enumerator;
	HRESULT result = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, &enumerator);

	if(FAILED(result)) {
		return;
	}

	IMMDevice* device;
	result = enumerator->lpVtbl->GetDevice(enumerator, identifier, &device);
	enumerator->lpVtbl->Release(enumerator);

	if(FAILED(result)) {
		return;
	}

	IAudioClient* audioClient;
	result = device->lpVtbl->Activate(device, &IID_IAudioClient, CLSCTX_ALL, NULL, &audioClient);
	device->lpVtbl->Release(device);

	if(FAILED(result)) {
		return;
	}

	WAVEFORMATEX* format;
	result = audioClient->lpVtbl->GetMixFormat(audioClient, &format);

	if(FAILED(result)) {
		goto releaseAudioClient;
	}

	result = audioClient->lpVtbl->Initialize(audioClient, AUDCLNT_SHAREMODE_SHARED, 0, 10000000, 0, format, NULL);

	if(FAILED(result)) {
		goto freeFormat;
	}

	IAudioCaptureClient* client;
	result = audioClient->lpVtbl->GetService(audioClient, &IID_IAudioCaptureClient, &client);

	if(FAILED(result)) {
		goto freeFormat;
	}

	result = audioClient->lpVtbl->Start(audioClient);

	if(FAILED(result)) {
		goto releaseClient;
	}

	for(size_t i = 0; i < 100; i++) {
		Sleep(50);
		PBYTE data;
		UINT32 frames;
		DWORD flags;
		result = client->lpVtbl->GetBuffer(client, &data, &frames, &flags, NULL, NULL);

		if(FAILED(result)) {
			goto stopClient;
		}

		printf("Frames: %lu\n", frames);
		result = client->lpVtbl->ReleaseBuffer(client, frames);

		if(FAILED(result)) {
			goto stopClient;
		}
	}
stopClient:
	result = audioClient->lpVtbl->Stop(audioClient);

	if(FAILED(result)) {
		goto releaseClient;
	}
releaseClient:
	client->lpVtbl->Release(client);
freeFormat:
	CoTaskMemFree(format);
releaseAudioClient:
	audioClient->lpVtbl->Release(audioClient);
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
		case AM_QUERY_AUDIO_CAPTURE:
			captureAudio((LPWSTR) message.lParam);
			KHOPAN_DEALLOCATE(message.lParam);
			break;
		}
	}
uninitialize:
	CoUninitialize();
	return 0;
}*/
