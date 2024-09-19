#include "libkhopanjava.h"

static void systemPrintln(JNIEnv* environment, const LPCWSTR message, const LPCSTR field) {
	jclass systemClass = (*environment)->FindClass(environment, "java/lang/System");

	if(!systemClass) {
		return;
	}

	jclass printStreamClass = (*environment)->FindClass(environment, "java/io/PrintStream");

	if(!printStreamClass) {
		return;
	}

	jfieldID outputField = (*environment)->GetStaticFieldID(environment, systemClass, field, "Ljava/io/PrintStream;");

	if(!outputField) {
		return;
	}

	jmethodID printlnMethod = (*environment)->GetMethodID(environment, printStreamClass, "println", "(Ljava/lang/String;)V");

	if(!printlnMethod) {
		return;
	}

	jobject output = (*environment)->GetStaticObjectField(environment, systemClass, outputField);

	if(!output) {
		return;
	}

	jstring string = (*environment)->NewString(environment, message, (jsize) wcslen(message));

	if(!string) {
		return;
	}

	(*environment)->CallVoidMethod(environment, output, printlnMethod, string);
}

void KHOPANJavaStandardOutput(JNIEnv* environment, const LPCWSTR message) {
	systemPrintln(environment, message, "out");
}

void KHOPANJavaStandardError(JNIEnv* environment, const LPCWSTR message) {
	systemPrintln(environment, message, "err");
}

void KHOPANJavaThrow(JNIEnv* environment, const LPCSTR class, const LPCWSTR message) {
	if(!environment || !class || !message) {
		return;
	}

	jclass classObject = (*environment)->FindClass(environment, class);

	if(!classObject) {
		return;
	}

	jmethodID constructor = (*environment)->GetMethodID(environment, classObject, "<init>", "(Ljava/lang/String;)V");

	if(!constructor) {
		return;
	}

	jobject object = (*environment)->NewObject(environment, classObject, constructor, (*environment)->NewString(environment, message, (jsize) wcslen(message)));

	if(!object) {
		return;
	}

	(*environment)->Throw(environment, (jthrowable) object);
}
