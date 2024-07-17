#include <WS2tcpip.h>
#include <khopanjava.h>
#include "exception.h"

_declspec(dllexport) void __stdcall ConnectHRSPServer(JNIEnv* const environment, LPCSTR hostName, LPCSTR port, const jobject callback) {
	jclass consumerClass = (*environment)->FindClass(environment, "java/util/function/Consumer");
	
	if(!consumerClass) {
		return;
	}

	jmethodID acceptMethod = (*environment)->GetMethodID(environment, consumerClass, "accept", "(Ljava/lang/Object;)V");

	if(!acceptMethod) {
		return;
	}

	WSADATA windowsSocketData;
	int status = WSAStartup(MAKEWORD(2, 2), &windowsSocketData);

	if(status) {
		SetLastError(status);
		HackontrolThrowWin32Error(environment, L"WSAStartup");
		return;
	}

	struct addrinfo hints = {0};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	struct addrinfo* result;
	status = getaddrinfo(hostName, port, &hints, &result);

	if(status) {
		SetLastError(status);
		HackontrolThrowWin32Error(environment, L"getaddrinfo");
		goto wsaCleanup;
	}

	SOCKET clientSocket = INVALID_SOCKET;

	for(struct addrinfo* pointer = result; pointer != NULL; pointer = pointer->ai_next) {
		clientSocket = socket(pointer->ai_family, pointer->ai_socktype, pointer->ai_protocol);

		if(clientSocket == INVALID_SOCKET) {
			SetLastError(WSAGetLastError());
			HackontrolThrowWin32Error(environment, L"socket");
			freeaddrinfo(result);
			goto wsaCleanup;
		}

		status = connect(clientSocket, pointer->ai_addr, (int) pointer->ai_addrlen);

		if(status != SOCKET_ERROR) {
			break;
		}

		closesocket(clientSocket);
		clientSocket = INVALID_SOCKET;
	}

	freeaddrinfo(result);

	if(clientSocket == INVALID_SOCKET) {
		KHJavaThrowInternalErrorW(environment, L"Unable to connect to the server");
		goto wsaCleanup;
	}

	const char* header = "HRSP 1.0 CONNECT";
	status = send(clientSocket, header, (int) strlen(header), 0);

	if(status == SOCKET_ERROR) {
		SetLastError(WSAGetLastError());
		HackontrolThrowWin32Error(environment, L"send");
		goto closeSocket;
	}

	char buffer[12];
	status = recv(clientSocket, buffer, 11, 0);

	if(status == SOCKET_ERROR) {
		SetLastError(WSAGetLastError());
		HackontrolThrowWin32Error(environment, L"recv");
		goto closeSocket;
	}

	buffer[11] = 0;

	if(strcmp(buffer, "HRSP 1.0 OK")) {
		KHJavaThrowInternalErrorW(environment, L"Server responded with an invalid response");
		goto closeSocket;
	}

	status = shutdown(clientSocket, SD_RECEIVE);

	if(status == SOCKET_ERROR) {
		SetLastError(WSAGetLastError());
		HackontrolThrowWin32Error(environment, L"shutdown");
		goto closeSocket;
	}

	(*environment)->CallObjectMethod(environment, callback, acceptMethod, (*environment)->NewStringUTF(environment, "**Connected**"));
closeSocket:
	closesocket(clientSocket);
wsaCleanup:
	WSACleanup();
}
