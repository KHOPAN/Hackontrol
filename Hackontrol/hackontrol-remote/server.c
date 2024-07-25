#include <WS2tcpip.h>
#include <khopanwin32.h>
#include "server.h"

#define REMOTE_PORT "42485"

DWORD WINAPI ServerThread(_In_ LPVOID parameter) {
	WSADATA windowsSocketData;
	int status = WSAStartup(MAKEWORD(2, 2), &windowsSocketData);
	int returnValue = 1;

	if(status) {
		RemoteError(status, L"WSAStartup");
		goto exit;
	}

	struct addrinfo hints = {0};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	struct addrinfo* result;
	status = getaddrinfo(NULL, REMOTE_PORT, &hints, &result);

	if(status) {
		RemoteError(status, L"getaddrinfo");
		goto wsaCleanup;
	}

	SOCKET serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if(serverSocket == INVALID_SOCKET) {
		RemoteError(WSAGetLastError(), L"socket");
		freeaddrinfo(result);
		goto wsaCleanup;
	}

	status = bind(serverSocket, result->ai_addr, (int) result->ai_addrlen);
	freeaddrinfo(result);

	if(status == SOCKET_ERROR) {
		RemoteError(WSAGetLastError(), L"bind");
		goto closeServerSocket;
	}

	if(listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
		RemoteError(WSAGetLastError(), L"listen");
		goto closeServerSocket;
	}

	SOCKET clientSocket = accept(serverSocket, NULL, NULL);

	if(clientSocket == INVALID_SOCKET) {
		RemoteError(WSAGetLastError(), L"accept");
		goto closeServerSocket;
	}

	RemoteAddListEntry();
	MessageBoxW(NULL, L"Connected", L"Hackontrol Remote", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	returnValue = 0;
	closesocket(clientSocket);
closeServerSocket:
	closesocket(serverSocket);
wsaCleanup:
	if(WSACleanup() == SOCKET_ERROR) {
		RemoteError(status, L"WSACleanup");
		returnValue = 1;
	}
exit:
	ExitRemote(returnValue);
	return returnValue;
}
