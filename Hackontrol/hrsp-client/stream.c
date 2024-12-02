#include <mfapi.h>
#include <mfidl.h>
#include "hrsp_client_internal.h"

void StreamRequestDevice(const SOCKET socket) {
	IMFAttributes* attributes;

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
	CoTaskMemFree(activates);
}
