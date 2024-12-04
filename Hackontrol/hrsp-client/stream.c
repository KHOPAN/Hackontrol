#include <libkhopanlist.h>
#include <mfapi.h>
#include <mfidl.h>
#include <hrsp_remote.h>
#include "hrsp_client_internal.h"

static BOOLEAN requestMonitors(const PDATASTREAM stream) {
	return TRUE;
}

static BOOLEAN requestCameras(const PDATASTREAM stream) {
	return TRUE;
}

void StreamRequestDevice(const SOCKET socket, const PHRSPDATA data) {
	DATASTREAM stream = {0};
	HRSPPACKET packet;
	packet.type = HRSP_REMOTE_CLIENT_RESPONSE_STREAM_DEVICE;
	BYTE byte = 0;

	if(!KHOPANStreamAdd(&stream, &byte, 1, NULL)) {
		goto functionExit;
	}

	if(!requestMonitors(&stream)) {
		goto functionExit;
	}

	if(!requestCameras(&stream)) {
		goto functionExit;
	}

	packet.size = stream.size;
	packet.data = stream.data;
	HRSPPacketSend(data, &packet, NULL);
	KHOPANStreamFree(&stream, NULL);
	return;
functionExit:
	KHOPANStreamFree(&stream, NULL);
	packet.size = 1;
	byte = 1;
	packet.data = &byte;
	HRSPPacketSend(data, &packet, NULL);
	/*IMFAttributes* attributes;

	if(FAILED(MFCreateAttributes(&attributes, 1))) {
		return;
	}

	if(FAILED(attributes->lpVtbl->SetGUID(attributes, &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID))) {
		attributes->lpVtbl->Release(attributes);
		return;
	}

	IMFActivate** activates;
	UINT32 count;
	HRESULT result = MFEnumDeviceSources(attributes, &activates, &count);
	attributes->lpVtbl->Release(attributes);

	if(FAILED(result)) {
		return;
	}

	printf("Count: %lu\n", count);
	CoTaskMemFree(activates);*/
}
