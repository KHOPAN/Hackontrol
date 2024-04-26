#include <jni.h>
#include "definition.h"
#include "utilities.h"

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* virtualMachine, void* reserved) {
	JNIEnv* environment = NULL;
	(*virtualMachine)->GetEnv(virtualMachine, (void**) &environment, JNI_VERSION_21);

	if(!environment) {
		return JNI_VERSION_21;
	}

	System_out_println(environment, "Hello world, from Native Library!");
	/*BOOL result = ProtectProcess();

	if(!result) {
		System_err_println(environment, "Process protection has failed!");
	}*/

	return JNI_VERSION_21;
}
