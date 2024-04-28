#include <Windows.h>
#include "khopanjni.h"
#include "khopanerror.h"

unsigned int print(JNIEnv* environment, const char* message, const char* fieldName) {
	jclass systemClass = (*environment)->FindClass(environment, "java/lang/System");

	if(!systemClass) {
		(*environment)->ExceptionClear(environment);
		return 1;
	}

	jclass printStreamClass = (*environment)->FindClass(environment, "java/io/PrintStream");

	if(!printStreamClass) {
		(*environment)->ExceptionClear(environment);
		return 2;
	}

	jfieldID outputField = (*environment)->GetStaticFieldID(environment, systemClass, fieldName, "Ljava/io/PrintStream;");

	if(!outputField) {
		(*environment)->ExceptionClear(environment);
		return 3;
	}

	jmethodID printlnMethod = (*environment)->GetMethodID(environment, printStreamClass, "println", "(Ljava/lang/String;)V");

	if(!printlnMethod) {
		(*environment)->ExceptionClear(environment);
		return 4;
	}

	jobject output = (*environment)->GetStaticObjectField(environment, systemClass, outputField);

	if(!output) {
		return 5;
	}

	jstring string = (*environment)->NewStringUTF(environment, message);

	if(!string) {
		return 6;
	}

	(*environment)->CallVoidMethod(environment, output, printlnMethod, string);
	return 0;
}

unsigned int KHStandardOutput(JNIEnv* environment, const char* message) {
	return print(environment, message, "out");
}

unsigned int KHStandardError(JNIEnv* environment, const char* message) {
	return print(environment, message, "err");
}

unsigned int KHWin32Error(JNIEnv* environment, unsigned long errorCode, const char* functionName) {
	char* message = KHGetWin32ErrorMessageA(errorCode, functionName);

	if(!message) {
		return 7;
	}

	int status = KHStandardError(environment, message);
	free(message);
	return status;
}
