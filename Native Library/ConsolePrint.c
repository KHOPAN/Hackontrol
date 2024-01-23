#include "utilities.h"

BOOL systemPrintInternal(JNIEnv* environment, const char* message, const char* fieldName) {
	jclass systemClass = (*environment)->FindClass(environment, "java/lang/System");
	
	if(!systemClass) {
		(*environment)->ExceptionClear(environment);
		return FALSE;
	}

	jclass printStreamClass = (*environment)->FindClass(environment, "java/io/PrintStream");

	if(!printStreamClass) {
		(*environment)->ExceptionClear(environment);
		return FALSE;
	}

	jfieldID errorField = (*environment)->GetStaticFieldID(environment, systemClass, fieldName, "Ljava/io/PrintStream;");

	if(!errorField) {
		(*environment)->ExceptionClear(environment);
		return FALSE;
	}

	jmethodID printlnMethod = (*environment)->GetMethodID(environment, printStreamClass, "println", "(Ljava/lang/String;)V");
	
	if(!printlnMethod) {
		(*environment)->ExceptionClear(environment);
		return FALSE;
	}
	
	jobject error = (*environment)->GetStaticObjectField(environment, systemClass, errorField);

	if(!error) {
		return FALSE;
	}

	jstring string = (*environment)->NewStringUTF(environment, message);

	if(!string) {
		return FALSE;
	}

	(*environment)->CallVoidMethod(environment, error, printlnMethod, string);
	return TRUE;
}

BOOL System_out_println(JNIEnv* environment, const char* message) {
	return systemPrintInternal(environment, message, "out");
}

BOOL System_err_println(JNIEnv* environment, const char* message) {
	return systemPrintInternal(environment, message, "err");
}
