#include <khopanjava.h>
#include "Kernel.h"
#include "exception.h"

typedef void(__stdcall* ConnectHRSPServerFunction) (JNIEnv* const environment, LPCSTR hostName, const unsigned int port, const jobject callback);

void Kernel_connectHRSPServer(JNIEnv* const environment, const jclass class, const jstring host, const jint port, const jobject callback) {
	if(!host) {
		KHJavaThrowIllegalArgumentExceptionW(environment, L"Host cannot be null");
		return;
	}

	if(!callback) {
		KHJavaThrowIllegalArgumentExceptionW(environment, L"Callback cannot be null");
		return;
	}

	HMODULE library = LoadLibraryW(L"D:\\GitHub Repository\\Hackontrol\\Hackontrol\\x64\\Debug\\libhrsp32.dll");

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

	if(!hostName || !strlen(hostName)) {
		KHJavaThrowIllegalArgumentExceptionW(environment, L"Null or empty host name");
		return;
	}

	function(environment, hostName, port, callback);
}
