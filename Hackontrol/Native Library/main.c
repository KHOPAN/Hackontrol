#include <jni.h>
#include "definition.h"
#include "utilities.h"

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* virtualMachine, void* reserved) {
	JNIEnv* environment = NULL;
	(*virtualMachine)->GetEnv(virtualMachine, (void**) &environment, JNI_VERSION_21);

	if(!environment) {
		return JNI_VERSION_21;
	}

	jclass hackontrolClass = (*environment)->FindClass(environment, "com/khopan/hackontrol/Hackontrol");

	if(!hackontrolClass) {
		(*environment)->ExceptionClear(environment);
		return JNI_VERSION_21;
	}

	jfieldID criticalProcessField = (*environment)->GetStaticFieldID(environment, hackontrolClass, "CRITICAL_PROCESS", "Z");
	
	if(!criticalProcessField) {
		(*environment)->ExceptionClear(environment);
		return JNI_VERSION_21;
	}

	jboolean criticalProcess = (*environment)->GetStaticBooleanField(environment, hackontrolClass, criticalProcessField);
	
	if(criticalProcess == FALSE) {
		return JNI_VERSION_21;
	}

	BOOL result = ProtectProcess();

	if(!result) {
		System_err_println(environment, "Protecting the process has failed!");
	}

	return JNI_VERSION_21;
}
