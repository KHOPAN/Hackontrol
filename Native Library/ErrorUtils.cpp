#include "ErrorUtils.h"

void ErrorUtils_ThrowException(JNIEnv* environment, const std::string& exceptionClassName, const std::string& message) {
	jclass exceptionClass = environment->FindClass(exceptionClassName.c_str());

	if(!exceptionClass) {
		environment->FatalError(message.c_str());
		return;
	}

	jmethodID exceptionConstructor = environment->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
	
	if(!exceptionConstructor) {
		environment->FatalError(message.c_str());
		return;
	}

	jstring string = environment->NewStringUTF(message.c_str());
	jobject exceptionInstance = environment->NewObject(exceptionClass, exceptionConstructor, string);
	jthrowable throwable = static_cast<jthrowable>(exceptionInstance);
	environment->Throw(throwable);
}
