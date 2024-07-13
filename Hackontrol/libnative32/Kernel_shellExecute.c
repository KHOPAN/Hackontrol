#include <khopanjava.h>
#include "Kernel.h"

void Kernel_shellExecute(JNIEnv* const environment, const jclass class, const jstring command, const jobject callback) {
	if(!command) {
		return;
	}

	if(!callback) {
		KHJavaThrowIllegalArgumentExceptionW(environment, L"Callback cannot be null");
		return;
	}

	jclass consumerClass = (*environment)->FindClass(environment, "java/util/function/Consumer");

	if(!consumerClass) {
		return;
	}

	jmethodID acceptMethod = (*environment)->GetMethodID(environment, consumerClass, "accept", "(Ljava/lang/Object;)V");

	if(!acceptMethod) {
		return;
	}

	(*environment)->CallVoidMethod(environment, callback, acceptMethod, NULL);
}
