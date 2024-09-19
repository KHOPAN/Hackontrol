#include "libkhopanjava.h"

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
