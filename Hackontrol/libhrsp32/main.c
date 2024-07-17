#include <WinSock2.h>
#include <WS2tcpip.h>
#include <khopanjava.h>
#include "exception.h"

_declspec(dllexport) void __stdcall ConnectHRSPServer(JNIEnv* const environment, LPCSTR hostName, const unsigned int port, const jobject callback) {
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
	status = getaddrinfo("localhost", "42485", &hints, &result);

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

		status = connect(clientSocket, pointer->ai_addr, pointer->ai_addrlen);

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

	closesocket(clientSocket);
wsaCleanup:
	WSACleanup();
}
