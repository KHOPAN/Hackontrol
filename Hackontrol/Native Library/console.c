#include "console.h"

int systemPrintInternal(JNIEnv* environment, const char* message, const char* fieldName) {
	jclass systemClass = (*environment)->FindClass(environment, "java/lang/System");
	
	if(!systemClass) {
		(*environment)->ExceptionClear(environment);
		return 0;
	}

	jclass printStreamClass = (*environment)->FindClass(environment, "java/io/PrintStream");

	if(!printStreamClass) {
		(*environment)->ExceptionClear(environment);
		return 0;
	}

	jfieldID errorField = (*environment)->GetStaticFieldID(environment, systemClass, fieldName, "Ljava/io/PrintStream;");

	if(!errorField) {
		(*environment)->ExceptionClear(environment);
		return 0;
	}

	jmethodID printlnMethod = (*environment)->GetMethodID(environment, printStreamClass, "println", "(Ljava/lang/String;)V");
	
	if(!printlnMethod) {
		(*environment)->ExceptionClear(environment);
		return 0;
	}
	
	jobject error = (*environment)->GetStaticObjectField(environment, systemClass, errorField);

	if(!error) {
		return 0;
	}

	jstring string = (*environment)->NewStringUTF(environment, message);

	if(!string) {
		return 0;
	}

	(*environment)->CallVoidMethod(environment, error, printlnMethod, string);
	return 1;
}

int System_out_println(JNIEnv* environment, const char* message) {
	return systemPrintInternal(environment, message, "out");
}

int System_err_println(JNIEnv* environment, const char* message) {
	return systemPrintInternal(environment, message, "err");
}
