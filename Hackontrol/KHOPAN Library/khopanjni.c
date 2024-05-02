#include "khopanjni.h"
#include "khopanerror.h"

void print(JNIEnv* environment, const LPVOID message, const LPSTR fieldName, BOOL wide) {
	jclass systemClass = (*environment)->FindClass(environment, "java/lang/System");

	if(!systemClass) {
		(*environment)->ExceptionClear(environment);
		return;
	}

	jclass printStreamClass = (*environment)->FindClass(environment, "java/io/PrintStream");

	if(!printStreamClass) {
		(*environment)->ExceptionClear(environment);
		return;
	}

	jfieldID outputField = (*environment)->GetStaticFieldID(environment, systemClass, fieldName, "Ljava/io/PrintStream;");

	if(!outputField) {
		(*environment)->ExceptionClear(environment);
		return;
	}

	jmethodID printlnMethod = (*environment)->GetMethodID(environment, printStreamClass, "println", "(Ljava/lang/String;)V");

	if(!printlnMethod) {
		(*environment)->ExceptionClear(environment);
		return;
	}

	jobject output = (*environment)->GetStaticObjectField(environment, systemClass, outputField);

	if(!output) {
		return;
	}

	jstring string;

	if(wide) {
		LPWSTR wideString = (LPWSTR) message;
		string = (*environment)->NewString(environment, wideString, (jsize) wcslen(wideString));
	} else {
		string = (*environment)->NewStringUTF(environment, (LPSTR) message);
	}

	if(!string) {
		return;
	}

	(*environment)->CallVoidMethod(environment, output, printlnMethod, string);
}

void KHStandardOutputA(JNIEnv* environment, const LPSTR message) {
	print(environment, message, "out", FALSE);
}

void KHStandardOutputW(JNIEnv* environment, const LPWSTR message) {
	print(environment, message, "out", TRUE);
}

void KHStandardErrorA(JNIEnv* environment, const LPSTR message) {
	print(environment, message, "err", FALSE);
}

void KHStandardErrorW(JNIEnv* environment, const LPWSTR message) {
	print(environment, message, "err", TRUE);
}

void KHWin32ErrorA(JNIEnv* environment, DWORD errorCode, const LPSTR functionName) {
	LPSTR message = KHGetWin32ErrorMessageA(errorCode, functionName);

	if(!message) {
		return;
	}

	KHStandardErrorA(environment, message);
	LocalFree(message);
}

void KHWin32ErrorW(JNIEnv* environment, DWORD errorCode, const LPWSTR functionName) {
	LPWSTR message = KHGetWin32ErrorMessageW(errorCode, functionName);

	if(!message) {
		return;
	}

	KHStandardErrorW(environment, message);
	LocalFree(message);
}
