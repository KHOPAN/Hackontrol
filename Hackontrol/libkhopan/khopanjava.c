#include "khopanwin32.h"
#include "khopanstring.h"
#include "khopanjava.h"

static void print(JNIEnv* environment, const LPVOID message, const LPSTR fieldName, BOOL wide) {
	jclass systemClass = (*environment)->FindClass(environment, "java/lang/System");

	if(!systemClass) {
		return;
	}

	jclass printStreamClass = (*environment)->FindClass(environment, "java/io/PrintStream");

	if(!printStreamClass) {
		return;
	}

	jfieldID outputField = (*environment)->GetStaticFieldID(environment, systemClass, fieldName, "Ljava/io/PrintStream;");

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

	jstring string = wide ? KHJavaFromNativeStringW(environment, (LPWSTR) message) : KHJavaFromNativeStringA(environment, (LPSTR) message);

	if(!string) {
		return;
	}

	(*environment)->CallVoidMethod(environment, output, printlnMethod, string);
}

void KHJavaStandardOutputA(JNIEnv* environment, const LPSTR message) {
	print(environment, message, "out", FALSE);
}

void KHJavaStandardOutputW(JNIEnv* environment, const LPWSTR message) {
	print(environment, message, "out", TRUE);
}

void KHJavaStandardErrorA(JNIEnv* environment, const LPSTR message) {
	print(environment, message, "err", FALSE);
}

void KHJavaStandardErrorW(JNIEnv* environment, const LPWSTR message) {
	print(environment, message, "err", TRUE);
}

void KHJavaWin32ErrorA(JNIEnv* environment, DWORD errorCode, const LPSTR functionName) {
	LPWSTR buffer = KHWIN32_MESSAGE(errorCode, functionName);

	if(!buffer) {
		return;
	}

	LPSTR message = KHFormatMessageA("%ws");
	LocalFree(buffer);

	if(!message) {
		return;
	}

	KHJavaStandardErrorA(environment, message);
	LocalFree(message);
}

void KHJavaWin32ErrorW(JNIEnv* environment, DWORD errorCode, const LPWSTR functionName) {
	LPWSTR message = KHWIN32_MESSAGE(errorCode, functionName);

	if(!message) {
		return;
	}

	KHJavaStandardErrorW(environment, message);
	LocalFree(message);
}

LPSTR KHJavaToNativeStringA(JNIEnv* environment, jstring string) {
	if(!string) {
		return NULL;
	}

	size_t size = (((size_t) (*environment)->GetStringUTFLength(environment, string)) + 1) * sizeof(CHAR);
	LPSTR buffer = LocalAlloc(LMEM_FIXED, size);

	if(!buffer) {
		return NULL;
	}

	if(size > 0) {
		const char* javaString = (*environment)->GetStringUTFChars(environment, string, NULL);

		if(javaString) {
			memcpy_s(buffer, size, javaString, size);
			(*environment)->ReleaseStringUTFChars(environment, string, javaString);
		}
	}

	buffer[size - 1] = 0;
	return buffer;
}

LPWSTR KHJavaToNativeStringW(JNIEnv* environment, jstring string) {
	if(!string) {
		return NULL;
	}

	size_t size = (((size_t) (*environment)->GetStringLength(environment, string)) + 1) * sizeof(WCHAR);
	LPWSTR buffer = LocalAlloc(LMEM_FIXED, size);

	if(!buffer) {
		return NULL;
	}

	if(size > 0) {
		const wchar_t* javaString = (*environment)->GetStringChars(environment, string, NULL);

		if(javaString) {
			memcpy_s(buffer, size, javaString, size);
			(*environment)->ReleaseStringChars(environment, string, javaString);
		}
	}

	buffer[size - 1] = 0;
	return buffer;
}

jstring KHJavaFromNativeStringA(JNIEnv* environment, const LPSTR string) {
	return (*environment)->NewStringUTF(environment, string);
}

jstring KHJavaFromNativeStringW(JNIEnv* environment, const LPWSTR string) {
	size_t length = wcslen(string);
	return (*environment)->NewString(environment, string, (jsize) length);
}