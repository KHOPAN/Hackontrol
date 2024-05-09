#include <khopanjava.h>
#include "native.h"
#include <endpointvolume.h>
#include <mmdeviceapi.h>

EXTERN_GUID(CLSID_MMDeviceEnumerator, 0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E);
EXTERN_GUID(IID_IMMDeviceEnumerator, 0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6);
EXTERN_GUID(IID_IAudioEndpointVolume, 0x5CDF2C82, 0x841E, 0x4546, 0x97, 0x22, 0x0C, 0xF7, 0x40, 0x78, 0x22, 0x9A);

jboolean Native_volume(JNIEnv* environment, jclass nativeLibraryClass, jfloat volume) {
	HRESULT result = CoInitialize(NULL);

	if(FAILED(result)) {
		KHJavaWin32ErrorW(environment, result, L"CoInitialize");
		return FALSE;
	}

	IMMDeviceEnumerator* enumerator;
	result = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, &enumerator);
	int returnValue = FALSE;

	if(FAILED(result)) {
		KHJavaWin32ErrorW(environment, result, L"CoCreateInstance");
		goto uninitialize;
	}

	IMMDevice* device;
	result = enumerator->lpVtbl->GetDefaultAudioEndpoint(enumerator, eRender, eConsole, &device);
	enumerator->lpVtbl->Release(enumerator);

	if(FAILED(result)) {
		KHJavaWin32ErrorW(environment, result, L"IMMDeviceEnumerator::GetDefaultAudioEndpoint");
		goto uninitialize;
	}

	IAudioEndpointVolume* endpointVolume;
	result = device->lpVtbl->Activate(device, &IID_IAudioEndpointVolume, CLSCTX_ALL, NULL, &endpointVolume);
	device->lpVtbl->Release(device);

	if(FAILED(result)) {
		KHJavaWin32ErrorW(environment, result, L"IMMDevice::Activate");
		goto uninitialize;
	}

	result = endpointVolume->lpVtbl->SetMasterVolumeLevelScalar(endpointVolume, volume, NULL);
	endpointVolume->lpVtbl->Release(endpointVolume);

	if(FAILED(result)) {
		KHJavaWin32ErrorW(environment, result, L"IAudioEndpointVolume::SetMasterVolumeLevelScalar");
		goto uninitialize;
	}

	returnValue = TRUE;
uninitialize:
	CoUninitialize();
	return returnValue;
}

jfloat Native_currentVolume(JNIEnv* environment, jclass nativeLibraryClass) {
	HRESULT result = CoInitialize(NULL);

	if(FAILED(result)) {
		KHJavaWin32ErrorW(environment, result, L"CoInitialize");
		return FALSE;
	}

	IMMDeviceEnumerator* enumerator;
	result = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, &enumerator);
	jfloat returnValue = 0.0f;

	if(FAILED(result)) {
		KHJavaWin32ErrorW(environment, result, L"CoCreateInstance");
		goto uninitialize;
	}

	IMMDevice* device;
	result = enumerator->lpVtbl->GetDefaultAudioEndpoint(enumerator, eRender, eConsole, &device);
	enumerator->lpVtbl->Release(enumerator);

	if(FAILED(result)) {
		KHJavaWin32ErrorW(environment, result, L"IMMDeviceEnumerator::GetDefaultAudioEndpoint");
		goto uninitialize;
	}

	IAudioEndpointVolume* endpointVolume;
	result = device->lpVtbl->Activate(device, &IID_IAudioEndpointVolume, CLSCTX_ALL, NULL, &endpointVolume);
	device->lpVtbl->Release(device);

	if(FAILED(result)) {
		KHJavaWin32ErrorW(environment, result, L"IMMDevice::Activate");
		goto uninitialize;
	}

	float volume;
	result = endpointVolume->lpVtbl->GetMasterVolumeLevelScalar(endpointVolume, &volume);
	endpointVolume->lpVtbl->Release(endpointVolume);

	if(FAILED(result)) {
		KHJavaWin32ErrorW(environment, result, L"IAudioEndpointVolume::GetMasterVolumeLevelScalar");
		goto uninitialize;
	}

	returnValue = volume;
uninitialize:
	CoUninitialize();
	return returnValue;
}
