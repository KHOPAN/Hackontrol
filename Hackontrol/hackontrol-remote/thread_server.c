#include <WS2tcpip.h>
#include <khopanwin32.h>
#include "thread_server.h"
#include "logger.h"

#define REMOTE_PORT L"42485"

static SOCKET listenSocket;

DWORD WINAPI ServerThread(_In_ LPVOID parameter) {
	LOG("[Server Thread]: Hello from server thread\n");
	WSADATA socketData;
	int status = WSAStartup(MAKEWORD(2, 2), &socketData);

	if(status) {
		KHWin32DialogErrorW(status, L"WSAStartup");
		return 0;
	}

	LOG("[Server Thread]: WSA Description: %s\n" COMMA socketData.szDescription);
	ADDRINFOW hints = {0};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	PADDRINFOW result;
	status = GetAddrInfoW(NULL, REMOTE_PORT, &hints, &result);

	if(status) {
		KHWin32DialogErrorW(status, L"GetAddrInfoW");
		goto cleanup;
	}

	LOG("[Server Thread]: Starting the listening socket\n");
	listenSocket = WSASocketW(result->ai_family, result->ai_socktype, result->ai_protocol, NULL, 0, WSA_FLAG_OVERLAPPED);

	if(listenSocket == INVALID_SOCKET) {
		KHWin32DialogErrorW(WSAGetLastError(), L"WSASocketW");
		FreeAddrInfoW(result);
		goto cleanup;
	}

	status = bind(listenSocket, result->ai_addr, (int) result->ai_addrlen);
	FreeAddrInfoW(result);

	if(status == SOCKET_ERROR) {
		KHWin32DialogErrorW(WSAGetLastError(), L"bind");
		goto closeListenSocket;
	}

	if(listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		KHWin32DialogErrorW(WSAGetLastError(), L"listen");
		goto closeListenSocket;
	}

	LOG("[Server Thread]: Listening for incoming connection...\n");

	while(TRUE) {
		SOCKADDR_IN socketAddress;
		int socketAddressLength = sizeof(SOCKADDR_IN);
		SOCKET socket = accept(listenSocket, (struct sockaddr*) &socketAddress, &socketAddressLength);

		if(socket == INVALID_SOCKET) {
			int error = WSAGetLastError();

			if(error == WSAEINTR) {
				break;
			}

			KHWin32DialogErrorW(error, L"accept");
			continue;
		}

		WCHAR socketAddressBuffer[17];

		if(!InetNtopW(AF_INET, &socketAddress.sin_addr, socketAddressBuffer, 16)) {
			KHWin32DialogErrorW(WSAGetLastError(), L"InetNtopW");
			closesocket(socket);
			continue;
		}

		socketAddressBuffer[16] = 0;
		LOG("[Server Thread]: Client connected: %ws\n" COMMA socketAddressBuffer);
	}
closeListenSocket:
	closesocket(listenSocket);
cleanup:
	WSACleanup();
	return 0;
}

void ExitServerThread() {
	if(closesocket(listenSocket) == SOCKET_ERROR) {
		KHWin32DialogErrorW(WSAGetLastError(), L"closesocket");
	}
}
