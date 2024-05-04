#include <khopanjni.h>
#include <mfapi.h>
#include <Mfidl.h>
#include "native_camera.h"
#include "camera_internal.h"

jobjectArray NativeLibrary_cameraList(JNIEnv* environment, jclass nativeLibraryClass) {
	jclass cameraDeviceClass = (*environment)->FindClass(environment, "com/khopan/hackontrol/CameraDevice");
	
	if(!cameraDeviceClass) {
		KHStandardErrorW(environment, L"Class 'com.khopan.hackontrol.CameraDevice' not found");
		return NULL;
	}
	
	jmethodID cameraDeviceConstructor = (*environment)->GetMethodID(environment, cameraDeviceClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");
	
	if(!cameraDeviceConstructor) {
		KHStandardErrorW(environment, L"Constructor 'com.khopan.hackontrol.CameraDevice(Ljava/lang/String;Ljava/lang/String;)V' not found");
		return NULL;
	}

	if(!InitializeCamera(environment)) {
		return NULL;
	}

	IMFAttributes* attributes;
	HRESULT result = MFCreateAttributes(&attributes, 1);
	jobjectArray returnResult = NULL;

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"MFCreateAttributes");
		goto uninitialize;
	}

	result = attributes->lpVtbl->SetGUID(attributes, &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"IMFAttributes::SetGUID");
		goto releaseAttributes;
	}

	IMFActivate** devices;
	UINT32 count;
	result = MFEnumDeviceSources(attributes, &devices, &count);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"MFEnumDeviceSources");
		goto releaseAttributes;
	}

	if(count < 1) {
		returnResult = (*environment)->NewObjectArray(environment, 0, cameraDeviceClass, NULL);
		goto freeDevices;
	}

	jobjectArray objectArray = (*environment)->NewObjectArray(environment, count, cameraDeviceClass, NULL);

	if(!objectArray) {
		goto freeDevices;
	}

	for(UINT32 i = 0; i < count; i++) {
		IMFActivate* activate = devices[i];
		LPWSTR deviceName;
		UINT32 deviceNameLength;
		result = activate->lpVtbl->GetAllocatedString(activate, &MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &deviceName, &deviceNameLength);
	
		if(FAILED(result)) {
			KHWin32ErrorW(environment, result, L"IMFActivate::GetAllocatedString");
			result = activate->lpVtbl->Release(activate);

			if(FAILED(result)) {
				KHWin32ErrorW(environment, result, L"IMFActivate::Release");
			}

			goto freeDevices;
		}

		LPWSTR symbolicLink;
		UINT32 symbolicLinkLength;
		result = activate->lpVtbl->GetAllocatedString(activate, &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &symbolicLink, &symbolicLinkLength);

		if(FAILED(result)) {
			KHWin32ErrorW(environment, result, L"IMFActivate::GetAllocatedString");
			CoTaskMemFree(deviceName);
			result = activate->lpVtbl->Release(activate);

			if(FAILED(result)) {
				KHWin32ErrorW(environment, result, L"IMFActivate::Release");
			}

			goto freeDevices;
		}

		jstring deviceNameJava = (*environment)->NewString(environment, deviceName, (jsize) deviceNameLength);
		jstring symbolicLinkJava = (*environment)->NewString(environment, symbolicLink, (jsize) symbolicLinkLength);
		jobject cameraDeviceInstance = (*environment)->NewObject(environment, cameraDeviceClass, cameraDeviceConstructor, deviceNameJava, symbolicLinkJava);
		(*environment)->SetObjectArrayElement(environment, objectArray, (jsize) i, cameraDeviceInstance);
		CoTaskMemFree(deviceName);
		CoTaskMemFree(symbolicLink);
		result = activate->lpVtbl->Release(activate);

		if(FAILED(result)) {
			KHWin32ErrorW(environment, result, L"IMFActivate::Release");
			goto freeDevices;
		}
	}

	returnResult = objectArray;
freeDevices:
	CoTaskMemFree(devices);
releaseAttributes:
	result = attributes->lpVtbl->Release(attributes);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"IMFAttributes::Release");
	}
uninitialize:
	UninitializeCamera(environment);
	return returnResult;
}
