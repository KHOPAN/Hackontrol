#include <khopanstring.h>
#include <khopanjava.h>
#include <hrsp_client.h>
#include <jni.h>

typedef struct {
	JNIEnv* environment;
	jobject callback;
	jmethodID acceptMethod;
} CONNECTEDPARAMETER, *PCONNECTEDPARAMETER;

static void connected(PCONNECTEDPARAMETER parameter) {
	if(!parameter) {
		return;
	}

	(*parameter->environment)->CallObjectMethod(parameter->environment, parameter->callback, parameter->acceptMethod, (*parameter->environment)->NewStringUTF(parameter->environment, "**Connected**"));
}

_declspec(dllexport) void __stdcall ConnectHRSPServer(JNIEnv* const environment, LPCSTR hostName, LPCSTR port, const jobject callback) {
	jclass consumerClass = (*environment)->FindClass(environment, "java/util/function/Consumer");

	if(!consumerClass) {
		return;
	}

	jmethodID acceptMethod = (*environment)->GetMethodID(environment, consumerClass, "accept", "(Ljava/lang/Object;)V");

	if(!acceptMethod) {
		return;
	}

	PCONNECTEDPARAMETER parameter = LocalAlloc(LMEM_FIXED, sizeof(CONNECTEDPARAMETER));

	if(!parameter) {
		return;
	}

	parameter->environment = environment;
	parameter->callback = callback;
	parameter->acceptMethod = acceptMethod;
	LPWSTR serverAddress = KHFormatMessageW(L"%S", hostName);
	LPWSTR serverPort = KHFormatMessageW(L"%S", port);
	HRSPCLIENTINPUT input = {0};
	input.parameter = parameter;
	input.callbackConnected = connected;
	HRSPCLIENTERROR error;
	BOOL status = HRSPClientConnectToServer(serverAddress, serverPort, &input, &error);
	LocalFree(parameter);

	if(serverAddress) {
		LocalFree(serverAddress);
	}

	if(serverPort) {
		LocalFree(serverPort);
	}

	if(!status) {
		LPWSTR message = HRSPClientGetErrorMessage(L"HRSPClientConnectToServer", &error);

		if(message) {
			KHJavaThrowW(environment, "com/khopan/hackontrol/Win32Error", message);
			LocalFree(message);
		}
	}
}
