#include <jni.h>
#include "definition.h"

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* virtualMachine, void* reserved) {
	ProtectProcess();
	return JNI_VERSION_21;
}
