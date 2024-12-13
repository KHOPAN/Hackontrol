#include <libkhopanlist.h>
#include <mfapi.h>
#include <mfidl.h>
#include <hrsp_remote.h>
#include "hrsp_client_internal.h"

static BOOL iterateMonitor(HMONITOR monitor, const HDC context, const LPRECT bounds, const PDATASTREAM stream) {
	MONITORINFOEXW information;
	information.cbSize = sizeof(MONITORINFOEXW);

	if(!GetMonitorInfoW(monitor, (LPMONITORINFO) &information)) {
		return FALSE;
	}

	BYTE bytes[4];
	bytes[0] = information.dwFlags & MONITORINFOF_PRIMARY ? HRSP_REMOTE_STREAM_DEVICE_PRIMARY_MONITOR : HRSP_REMOTE_STREAM_DEVICE_MONITOR;

	if(!KHOPANStreamAdd(stream, bytes, 1, NULL)) {
		return FALSE;
	}

	size_t length = wcslen(information.szDevice) * sizeof(WCHAR);
	bytes[0] = (length >> 24) & 0xFF;
	bytes[1] = (length >> 16) & 0xFF;
	bytes[2] = (length >> 8) & 0xFF;
	bytes[3] = length & 0xFF;

	if(!KHOPANStreamAdd(stream, bytes, 4, NULL) || !KHOPANStreamAdd(stream, information.szDevice, length, NULL)) {
		return FALSE;
	}

	length = sizeof(HMONITOR);
	bytes[0] = (length >> 24) & 0xFF;
	bytes[1] = (length >> 16) & 0xFF;
	bytes[2] = (length >> 8) & 0xFF;
	bytes[3] = length & 0xFF;
	return KHOPANStreamAdd(stream, bytes, 4, NULL) && KHOPANStreamAdd(stream, &monitor, sizeof(HMONITOR), NULL);
}

static BOOLEAN addCamera(const PDATASTREAM stream, IMFActivate* activate) {
	if(!activate) {
		return TRUE;
	}

	BYTE bytes[4];
	bytes[0] = HRSP_REMOTE_STREAM_DEVICE_CAMERA;

	if(!KHOPANStreamAdd(stream, bytes, 1, NULL)) {
		goto functionExit;
	}

	LPWSTR string;
	UINT32 stringLength;

	if(FAILED(activate->lpVtbl->GetAllocatedString(activate, &MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &string, &stringLength))) {
		goto functionExit;
	}

	stringLength *= sizeof(WCHAR);
	bytes[0] = (stringLength >> 24) & 0xFF;
	bytes[1] = (stringLength >> 16) & 0xFF;
	bytes[2] = (stringLength >> 8) & 0xFF;
	bytes[3] = stringLength & 0xFF;

	if(!KHOPANStreamAdd(stream, bytes, 4, NULL)) {
		CoTaskMemFree(string);
		goto functionExit;
	}

	BOOLEAN result = KHOPANStreamAdd(stream, string, stringLength, NULL);
	CoTaskMemFree(string);

	if(!result || FAILED(activate->lpVtbl->GetAllocatedString(activate, &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &string, &stringLength))) {
		goto functionExit;
	}

	stringLength *= sizeof(WCHAR);
	bytes[0] = (stringLength >> 24) & 0xFF;
	bytes[1] = (stringLength >> 16) & 0xFF;
	bytes[2] = (stringLength >> 8) & 0xFF;
	bytes[3] = stringLength & 0xFF;

	if(!KHOPANStreamAdd(stream, bytes, 4, NULL)) {
		CoTaskMemFree(string);
		goto functionExit;
	}

	result = KHOPANStreamAdd(stream, string, stringLength, NULL);
	CoTaskMemFree(string);
	activate->lpVtbl->Release(activate);
	return result;
functionExit:
	activate->lpVtbl->Release(activate);
	return FALSE;
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

	BOOLEAN success = TRUE;

	for(UINT32 i = 0; i < count; i++) {
		if(!addCamera(stream, activates[i])) {
			success = FALSE;
		}
	}

	CoTaskMemFree(activates);
	return success;
}

void StreamRequestDevice(const SOCKET socket, const PHRSPDATA data) {
	DATASTREAM stream = {0};
	HRSPPACKET packet;
	packet.type = HRSP_REMOTE_CLIENT_RESPONSE_STREAM_DEVICE;
	BYTE byte = 0;

	if(!KHOPANStreamAdd(&stream, &byte, 1, NULL)) {
		goto functionExit;
	}

	if(!EnumDisplayMonitors(NULL, NULL, (MONITORENUMPROC) iterateMonitor, (LPARAM) &stream)) {
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
