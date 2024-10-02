#include <WS2tcpip.h>
#include "remote.h"

DWORD WINAPI ThreadServer(_In_ SOCKET* socketListen) {
	DWORD codeExit = 1;

	if(!socketListen) {
		LOG("[Server]: Empty thread parameter\n");
		goto functionExit;
	}

	LOG("[Server]: Initializing\n");
	ADDRINFOW hints = {0};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	int status = GetAddrInfoW(NULL, REMOTE_PORT, &hints, &hints.ai_next);

	if(status) {
		KHOPANERRORMESSAGE_WIN32(status, L"GetAddrInfoW");
		goto functionExit;
	}

	*socketListen = WSASocketW(hints.ai_next->ai_family, hints.ai_next->ai_socktype, hints.ai_next->ai_protocol, NULL, 0, WSA_FLAG_OVERLAPPED);

	if(*socketListen == INVALID_SOCKET) {
		KHOPANLASTERRORMESSAGE_WSA(L"WSASocketW");
		FreeAddrInfoW(hints.ai_next);
		goto functionExit;
	}

	status = bind(*socketListen, hints.ai_next->ai_addr, (int) hints.ai_next->ai_addrlen);
	FreeAddrInfoW(hints.ai_next);

	if(status == SOCKET_ERROR) {
		KHOPANLASTERRORMESSAGE_WSA(L"bind");
		goto functionExit;
	}

	if(listen(*socketListen, SOMAXCONN) == SOCKET_ERROR) {
		KHOPANLASTERRORMESSAGE_WSA(L"listen");
		goto functionExit;
	}

	LOG("[Server]: Listening socket started\n");

	while(TRUE) {
		SOCKADDR_IN address;
		status = sizeof(SOCKADDR_IN);
		SOCKET socket = accept(*socketListen, (struct sockaddr*) &address, &status);

		if(socket == INVALID_SOCKET) {
			if(WSAGetLastError() == WSAEINTR) break;
			KHOPANLASTERRORMESSAGE_WSA(L"accept");
			continue;
		}

		PCLIENT client = KHOPAN_ALLOCATE(sizeof(CLIENT));

		if(KHOPAN_ALLOCATE_FAILED(client)) {
			KHOPANERRORMESSAGE_WIN32(KHOPAN_ALLOCATE_ERROR, KHOPAN_ALLOCATE_FUNCTION);
			goto closeSocket;
		}

		if(!InetNtopW(AF_INET, &address.sin_addr, client->address, 16)) {
			KHOPANLASTERRORMESSAGE_WSA(L"InetNtopW");
			goto freeClient;
		}

		LOG("[Server]: Client: %ws\n", client->address);
	freeClient:
		KHOPAN_DEALLOCATE(client);
	closeSocket:
		closesocket(socket);
	}

	codeExit = 0;
functionExit:
	LOG("[Server]: Exit with code: %d\n", codeExit);
	return codeExit;
}
