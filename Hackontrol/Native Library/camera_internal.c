#include <combaseapi.h>
#include <mfapi.h>
#include <khopanjni.h>
#include "camera_internal.h"

BOOL InitializeCamera(JNIEnv* environment) {
	HRESULT result = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"CoInitializeEx");
		return FALSE;
	}

	result = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"MFStartup");
		return FALSE;
	}
	
	return TRUE;
}

BOOL UninitializeCamera(JNIEnv* environment) {
	HRESULT result = MFShutdown();

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"MFShutdown");
		return FALSE;
	}

	CoUninitialize();
	return TRUE;
}
