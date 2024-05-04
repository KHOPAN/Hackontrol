#include <khopanstring.h>
#include <khopanjni.h>
#include "native_camera.h"

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

	LPWSTR nativeString = KHJavaToNativeStringW(environment, symbolicLinkJava);

	if(!nativeString) {
		return NULL;
	}

	LPWSTR result = KHFormatMessageW(L"Native Symbolic Link: %ws", nativeString);
	LocalFree(nativeString);

	if(!result) {
		return NULL;
	}

	KHStandardOutputW(environment, result);
	LocalFree(result);
	return NULL;
}
