#include <khopanstring.h>
#include <khopanjava.h>
#include "Kernel.h"
#include "exception.h"

typedef void(__stdcall* ConnectHRSPServerFunction) (JNIEnv* const environment, LPCSTR hostName, LPCSTR port, const jobject callback);

void Kernel_connectHRSPServer(JNIEnv* const environment, const jclass class, const jstring host, const jint port, const jobject callback) {
	if(!host) {
		KHJavaThrowIllegalArgumentExceptionW(environment, L"Host name cannot be null");
		return;
	}

	if(!callback) {
		KHJavaThrowIllegalArgumentExceptionW(environment, L"Callback cannot be null");
		return;
	}

	jfieldID fieldPathFileLibHRSP32 = (*environment)->GetStaticFieldID(environment, class, "PathFileLibHRSP32", "Ljava/lang/String;");

	if(!fieldPathFileLibHRSP32) {
		return;
	}

	jstring pathFileLibHRSP32Instance = (*environment)->GetStaticObjectField(environment, class, fieldPathFileLibHRSP32);

	if(!pathFileLibHRSP32Instance) {
		KHJavaThrowInternalErrorW(environment, L"Field 'PathFileLibHRSP32' cannot be null");
		return;
	}

	const jchar* pathFileLibHRSP32 = (*environment)->GetStringChars(environment, pathFileLibHRSP32Instance, NULL);
	HMODULE library = LoadLibraryW(pathFileLibHRSP32);

	if(pathFileLibHRSP32) {
		(*environment)->ReleaseStringChars(environment, pathFileLibHRSP32Instance, pathFileLibHRSP32);
	}

	if(!library) {
		HackontrolThrowWin32Error(environment, L"LoadLibraryW");
		return;
	}

	ConnectHRSPServerFunction function = (ConnectHRSPServerFunction) GetProcAddress(library, "ConnectHRSPServer");

	if(!function) {
		HackontrolThrowWin32Error(environment, L"GetProcAddress");
		return;
	}

	const char* hostName = (*environment)->GetStringUTFChars(environment, host, NULL);

	if(!hostName) {
		KHJavaThrowIllegalArgumentExceptionW(environment, L"Host name cannot be null");
		return;
	}

	if(!strlen(hostName)) {
		KHJavaThrowIllegalArgumentExceptionW(environment, L"Host name cannot be empty");
		(*environment)->ReleaseStringUTFChars(environment, host, hostName);
		return;
	}

	LPSTR portName = KHFormatMessageA("%d", port);

	if(!portName) {
		SetLastError(ERROR_FUNCTION_FAILED);
		HackontrolThrowWin32Error(environment, L"KHFormatMessageA");
		(*environment)->ReleaseStringUTFChars(environment, host, hostName);
		return;
	}

	function(environment, hostName, portName, callback);
	LocalFree(portName);
	(*environment)->ReleaseStringUTFChars(environment, host, hostName);
}
