#include <combaseapi.h>
#include <mfapi.h>
#include <khopanjni.h>
#include "camera_internal.h"

void InitializeCamera(JNIEnv* environment) {
	HRESULT result = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"CoInitializeEx");
		return;
	}

	result = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"MFStartup");
	}
}

void UninitializeCamera(JNIEnv* environment) {
	HRESULT result = MFShutdown();

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"MFShutdown");
		return;
	}

	CoUninitialize();
}
