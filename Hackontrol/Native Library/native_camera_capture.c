#include <khopanjni.h>
#include <mfapi.h>
#include <Mfidl.h>
#include <mfreadwrite.h>
#include "native_camera.h"
#include "camera_internal.h"

jobject NativeLibrary_capture(JNIEnv* environment, jclass nativeLibraryClass, jobject cameraDeviceInstance) {
	if(!cameraDeviceInstance) {
		return NULL;
	}
	
	jclass cameraDeviceClass = (*environment)->FindClass(environment, "com/khopan/hackontrol/CameraDevice");

	if(!cameraDeviceClass) {
		KHStandardErrorW(environment, L"Class 'com.khopan.hackontrol.CameraDevice' not found");
		return NULL;
	}

	jfieldID symbolicLinkField = (*environment)->GetFieldID(environment, cameraDeviceClass, "symbolicLink", "Ljava/lang/String;");
	
	if(!symbolicLinkField) {
		KHStandardErrorW(environment, L"Field 'symbolicLink' not found in class 'com.khopan.hackontrol.CameraDevice'");
		return NULL;
	}

	jstring symbolicLinkJava = (jstring) ((*environment)->GetObjectField(environment, cameraDeviceInstance, symbolicLinkField));

	if(!symbolicLinkJava) {
		return NULL;
	}

	LPWSTR symbolicLink = KHJavaToNativeStringW(environment, symbolicLinkJava);

	if(!symbolicLink) {
		return NULL;
	}

	if(!InitializeCamera(environment)) {
		LocalFree(symbolicLink);
		return NULL;
	}

	IMFAttributes* attributes;
	HRESULT result = MFCreateAttributes(&attributes, 2);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"MFCreateAttributes");
		LocalFree(symbolicLink);
		goto uninitialize;
	}

	result = attributes->lpVtbl->SetGUID(attributes, &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"IMFAttributes::SetGUID");
		LocalFree(symbolicLink);
		goto releaseAttributes;
	}

	result = attributes->lpVtbl->SetString(attributes, &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, symbolicLink);
	LocalFree(symbolicLink);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"IMFAttributes::SetGUID");
		goto releaseAttributes;
	}

	IMFMediaSource* source;
	result = MFCreateDeviceSource(attributes, &source);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"MFCreateDeviceSource");
		goto releaseAttributes;
	}

	IMFSourceReader* reader;
	result = MFCreateSourceReaderFromMediaSource(source, NULL, &reader);
	HRESULT resultOverlapped = source->lpVtbl->Release(source);

	if(FAILED(resultOverlapped)) {
		KHWin32ErrorW(environment, resultOverlapped, L"IMFMediaSource::Release");
	}

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"MFCreateSourceReaderFromMediaSource");
		goto releaseAttributes;
	}

releaseReader:
	result = reader->lpVtbl->Release(reader);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"IMFSourceReader::Release");
	}
releaseAttributes:
	result = attributes->lpVtbl->Release(attributes);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"IMFAttributes::Release");
	}
uninitialize:
	UninitializeCamera(environment);
	return NULL;
}
