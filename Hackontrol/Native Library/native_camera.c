#include "native_camera.h"
#include "camera_internal.h"

jobject NativeLibrary_cameraList(JNIEnv* environment, jclass nativeLibraryClass) {
	InitializeCamera(environment);
	UninitializeCamera(environment);
	return NULL;
}
