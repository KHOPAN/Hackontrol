#include <stdlib.h>
#include <khopanwin32.h>
#include <endpointvolume.h>
#include <mmdeviceapi.h>

EXTERN_GUID(CLSID_MMDeviceEnumerator, 0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E);
EXTERN_GUID(IID_IMMDeviceEnumerator,  0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6);
EXTERN_GUID(IID_IAudioEndpointVolume, 0x5CDF2C82, 0x841E, 0x4546, 0x97, 0x22, 0x0C, 0xF7, 0x40, 0x78, 0x22, 0x9A);

int main(int argc, char** argv) {
	HRESULT result = CoInitialize(NULL);

	if(FAILED(result)) {
		KHWin32ConsoleErrorW(result, L"CoInitialize");
		return 1;
	}

	IMMDeviceEnumerator* enumerator;
	result = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, &enumerator);
	int returnValue = 1;

	if(FAILED(result)) {
		KHWin32ConsoleErrorW(result, L"CoCreateInstance");
		goto uninitialize;
	}

	IMMDevice* device;
	result = enumerator->lpVtbl->GetDefaultAudioEndpoint(enumerator, eRender, eConsole, &device);
	enumerator->lpVtbl->Release(enumerator);

	if(FAILED(result)) {
		KHWin32ConsoleErrorW(result, L"IMMDeviceEnumerator::GetDefaultAudioEndpoint");
		goto uninitialize;
	}

	IAudioEndpointVolume* volume;
	result = device->lpVtbl->Activate(device, &IID_IAudioEndpointVolume, CLSCTX_ALL, NULL, &volume);
	device->lpVtbl->Release(device);

	if(FAILED(result)) {
		KHWin32ConsoleErrorW(result, L"IMMDevice::Activate");
		goto uninitialize;
	}

	srand((unsigned int) GetTickCount64());
	unsigned long long count = 1000;

	while(count > 1) {
		float percentage = rand() / ((float) RAND_MAX);
		result = volume->lpVtbl->SetMasterVolumeLevelScalar(volume, percentage, NULL);

		if(FAILED(result)) {
			KHWin32ConsoleErrorW(result, L"IAudioEndpointVolume::SetMasterVolumeLevel");
			goto releaseVolume;
		}

		Sleep(10);
		count--;
	}

	returnValue = 0;
releaseVolume:
	volume->lpVtbl->Release(volume);
uninitialize:
	CoUninitialize();
	return returnValue;
}
