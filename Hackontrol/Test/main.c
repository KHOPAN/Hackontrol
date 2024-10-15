#include <libkhopan.h>
#include <mmdeviceapi.h>

EXTERN_GUID(CLSID_MMDeviceEnumerator, 0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E);
EXTERN_GUID(IID_IMMDeviceEnumerator,  0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6);

int main(int argc, char** argv) {
	HRESULT result = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"CoInitializeEx");
		return 1;
	}

	IMMDeviceEnumerator* enumerator;
	result = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, &enumerator);
	int codeExit = 1;

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"CoCreateInstance");
		goto uninitialize;
	}

	IMMDeviceCollection* collection;
	result = enumerator->lpVtbl->EnumAudioEndpoints(enumerator, eAll, DEVICE_STATEMASK_ALL, &collection);
	enumerator->lpVtbl->Release(enumerator);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"IMMDeviceEnumerator::EnumAudioEndpoints");
		goto uninitialize;
	}

	UINT count;
	result = collection->lpVtbl->GetCount(collection, &count);
	collection->lpVtbl->Release(collection);

	if(FAILED(result)) {
		KHOPANERRORMESSAGE_WIN32(result, L"IMMDeviceCollection::GetCount");
		goto uninitialize;
	}

	printf("Count: %u\n", count);
	codeExit = 0;
uninitialize:
	CoUninitialize();
	return codeExit;
}
