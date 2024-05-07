#include "khopanjava.h"

static void throwInternal(JNIEnv* environment, const LPSTR throwableClass, jstring string) {
	if(!string) {
		return;
	}

	jclass throwableClassObject = (*environment)->FindClass(environment, throwableClass);

	if(!throwableClassObject) {
		return;
	}

	jmethodID throwableConstructor = (*environment)->GetMethodID(environment, throwableClassObject, "<init>", "(Ljava/lang/String;)V");

	if(!throwableConstructor) {
		return;
	}

	jobject object = (*environment)->NewObject(environment, throwableClassObject, throwableConstructor, string);

	if(!object) {
		return;
	}

	(*environment)->Throw(environment, (jthrowable) object);
}

void KHJavaThrowA(JNIEnv* environment, const LPSTR throwableClass, const LPSTR message) {
	jstring string = KHJavaFromNativeStringA(environment, message);
	throwInternal(environment, throwableClass, string);
}

void KHJavaThrowW(JNIEnv* environment, const LPSTR throwableClass, const LPWSTR message) {
	jstring string = KHJavaFromNativeStringW(environment, message);
	throwInternal(environment, throwableClass, string);
}

void KHJavaThrowRuntimeExceptionA(JNIEnv* environment, const LPSTR message) {
	KHJavaThrowA(environment, "java/lang/RuntimeException", message);
}

void KHJavaThrowRuntimeExceptionW(JNIEnv* environment, const LPWSTR message) {
	KHJavaThrowW(environment, "java/lang/RuntimeException", message);
}

void KHJavaThrowIllegalArgumentExceptionA(JNIEnv* environment, const LPSTR message) {
	KHJavaThrowA(environment, "java/lang/IllegalArgumentException", message);
}

void KHJavaThrowIllegalArgumentExceptionW(JNIEnv* environment, const LPWSTR message) {
	KHJavaThrowW(environment, "java/lang/IllegalArgumentException", message);
}

void KHJavaThrowInternalErrorA(JNIEnv* environment, const LPSTR message) {
	KHJavaThrowA(environment, "java/lang/InternalError", message);
}

void KHJavaThrowInternalErrorW(JNIEnv* environment, const LPWSTR message) {
	KHJavaThrowW(environment, "java/lang/InternalError", message);
}
