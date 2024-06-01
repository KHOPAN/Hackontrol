#include <endpointvolume.h>
#include <mmdeviceapi.h>
#include "exception.h"
#include "User.h"

EXTERN_GUID(CLSID_MMDeviceEnumerator, 0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E);
EXTERN_GUID(IID_IMMDeviceEnumerator,  0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6);
EXTERN_GUID(IID_IAudioEndpointVolume, 0x5CDF2C82, 0x841E, 0x4546, 0x97, 0x22, 0x0C, 0xF7, 0x40, 0x78, 0x22, 0x9A);

static BOOL getAudioEndpointVolume(JNIEnv* const environment, IAudioEndpointVolume** const volume) {
	HRESULT result = CoInitialize(NULL);

	if(FAILED(result)) {
		SetLastError(result);
		HackontrolThrowWin32Error(environment, L"CoInitialize");
		return FALSE;
	}

	IMMDeviceEnumerator* enumerator;
	result = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, &enumerator);
	BOOL returnValue = FALSE;

	if(FAILED(result)) {
		SetLastError(result);
		HackontrolThrowWin32Error(environment, L"CoCreateInstance");
		goto uninitialize;
	}

	IMMDevice* device;
	result = enumerator->lpVtbl->GetDefaultAudioEndpoint(enumerator, eRender, eConsole, &device);
	enumerator->lpVtbl->Release(enumerator);

	if(FAILED(result)) {
		SetLastError(result);
		HackontrolThrowWin32Error(environment, L"IMMDeviceEnumerator::GetDefaultAudioEndpoint");
		goto uninitialize;
	}

	result = device->lpVtbl->Activate(device, &IID_IAudioEndpointVolume, CLSCTX_ALL, NULL, volume);
	device->lpVtbl->Release(device);

	if(FAILED(result)) {
		SetLastError(result);
		HackontrolThrowWin32Error(environment, L"IMMDevice::Activate");
		goto uninitialize;
	}

	returnValue = TRUE;
uninitialize:
	CoUninitialize();
	return returnValue;
}

jfloat User_getMasterVolume(JNIEnv* const environment, const jclass class) {
	IAudioEndpointVolume* endpoint;

	if(!getAudioEndpointVolume(environment, &endpoint)) {
		return 0.0f;
	}

	float volume;
	HRESULT result = endpoint->lpVtbl->GetMasterVolumeLevelScalar(endpoint, &volume);
	endpoint->lpVtbl->Release(endpoint);
	CoUninitialize();

	if(FAILED(result)) {
		SetLastError(result);
		HackontrolThrowWin32Error(environment, L"IAudioEndpointVolume::GetMasterVolumeLevelScalar");
		return 0.0f;
	}

	return volume;
}

jboolean User_isMute(JNIEnv* const environment, const jclass class) {
	IAudioEndpointVolume* endpoint;

	if(!getAudioEndpointVolume(environment, &endpoint)) {
		return FALSE;
	}

	BOOL mute;
	HRESULT result = endpoint->lpVtbl->GetMute(endpoint, &mute);
	endpoint->lpVtbl->Release(endpoint);
	CoUninitialize();

	if(FAILED(result)) {
		SetLastError(result);
		HackontrolThrowWin32Error(environment, L"IAudioEndpointVolume::GetMute");
		return FALSE;
	}

	return (jboolean) mute;
}

void User_setMasterVolume(JNIEnv* const environment, const jclass class, const jfloat volume) {
	IAudioEndpointVolume* endpoint;

	if(!getAudioEndpointVolume(environment, &endpoint)) {
		return;
	}

	HRESULT result = endpoint->lpVtbl->SetMasterVolumeLevelScalar(endpoint, volume, NULL);
	endpoint->lpVtbl->Release(endpoint);
	CoUninitialize();

	if(FAILED(result)) {
		SetLastError(result);
		HackontrolThrowWin32Error(environment, L"IAudioEndpointVolume::SetMasterVolumeLevelScalar");
	}
}

void User_setMute(JNIEnv* const environment, const jclass class, const jboolean mute) {
	IAudioEndpointVolume* endpoint;

	if(!getAudioEndpointVolume(environment, &endpoint)) {
		return;
	}

	HRESULT result = endpoint->lpVtbl->SetMute(endpoint, mute, NULL);
	endpoint->lpVtbl->Release(endpoint);
	CoUninitialize();

	if(FAILED(result)) {
		SetLastError(result);
		HackontrolThrowWin32Error(environment, L"IAudioEndpointVolume::SetMute");
	}
}
