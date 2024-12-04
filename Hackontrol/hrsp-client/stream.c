#include <libkhopanlist.h>
#include <mfapi.h>
#include <mfidl.h>
#include <hrsp_remote.h>
#include "hrsp_client_internal.h"

static BOOLEAN requestMonitors(const PDATASTREAM stream) {
	return TRUE;
}

static BOOLEAN addCamera(const PDATASTREAM stream, IMFActivate* activate) {
	BYTE bytes[4];
	bytes[0] = 1;

	if(!KHOPANStreamAdd(stream, bytes, 1, NULL)) {
		return FALSE;
	}

	LPWSTR string;
	UINT32 stringLength;

	if(FAILED(activate->lpVtbl->GetAllocatedString(activate, &MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &string, &stringLength))) {
		return FALSE;
	}

	stringLength *= sizeof(WCHAR);
	bytes[0] = (stringLength & 0xFF) << 24;
	bytes[1] = (stringLength & 0xFF) << 16;
	bytes[2] = (stringLength & 0xFF) << 8;
	bytes[3] = stringLength & 0xFF;

	if(!KHOPANStreamAdd(stream, &bytes, 4, NULL)) {
		CoTaskMemFree(string);
		return FALSE;
	}

	BOOLEAN result = KHOPANStreamAdd(stream, string, stringLength, NULL);
	CoTaskMemFree(string);

	if(!result || FAILED(activate->lpVtbl->GetAllocatedString(activate, &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &string, &stringLength))) {
		return FALSE;
	}

	stringLength *= sizeof(WCHAR);
	bytes[0] = (stringLength & 0xFF) << 24;
	bytes[1] = (stringLength & 0xFF) << 16;
	bytes[2] = (stringLength & 0xFF) << 8;
	bytes[3] = stringLength & 0xFF;

	if(!KHOPANStreamAdd(stream, &bytes, 4, NULL)) {
		CoTaskMemFree(string);
		return FALSE;
	}

	result = KHOPANStreamAdd(stream, string, stringLength, NULL);
	CoTaskMemFree(string);
	return result;
}

static BOOLEAN requestCameras(const PDATASTREAM stream) {
	IMFAttributes* attributes;

	if(FAILED(MFCreateAttributes(&attributes, 1))) {
		return FALSE;
	}

	if(FAILED(attributes->lpVtbl->SetGUID(attributes, &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID))) {
		attributes->lpVtbl->Release(attributes);
		return FALSE;
	}

	IMFActivate** activates;
	UINT32 count;
	HRESULT result = MFEnumDeviceSources(attributes, &activates, &count);
	attributes->lpVtbl->Release(attributes);

	if(FAILED(result)) {
		return FALSE;
	}

	BOOLEAN failed = FALSE;

	for(UINT32 i = 0; i < count; i++) {
		IMFActivate* activate = activates[i];
		if(!addCamera(stream, activate)) failed = TRUE;
		activate->lpVtbl->Release(activate);
	}

	CoTaskMemFree(activates);
	return !failed;
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
}
