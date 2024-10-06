#include <WS2tcpip.h>
#include <libkhopanlist.h>
#include "remote.h"

extern LINKEDLIST clientList;
extern HANDLE clientListMutex;

DWORD WINAPI ThreadServer(_In_ SOCKET* socketListen) {
	if(!socketListen) {
		LOG("[Server]: Empty thread parameter\n");
		return 1;
	}

	LOG("[Server]: Initializing\n");
	ARRAYLIST list;
	DWORD codeExit = 1;

	if(!KHOPANArrayInitialize(&list, sizeof(HANDLE))) {
		KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANArrayInitialize");
		goto functionExit;
	}

	ADDRINFOW hints = {0};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	int status = GetAddrInfoW(NULL, REMOTE_PORT, &hints, &hints.ai_next);

	if(status) {
		KHOPANERRORMESSAGE_WIN32(status, L"GetAddrInfoW");
		goto freeList;
	}

	*socketListen = WSASocketW(hints.ai_next->ai_family, hints.ai_next->ai_socktype, hints.ai_next->ai_protocol, NULL, 0, WSA_FLAG_OVERLAPPED);

	if(*socketListen == INVALID_SOCKET) {
		KHOPANLASTERRORMESSAGE_WSA(L"WSASocketW");
		FreeAddrInfoW(hints.ai_next);
		goto freeList;
	}

	status = bind(*socketListen, hints.ai_next->ai_addr, (int) hints.ai_next->ai_addrlen);
	FreeAddrInfoW(hints.ai_next);

	if(status == SOCKET_ERROR) {
		KHOPANLASTERRORMESSAGE_WSA(L"bind");
		goto freeList;
	}

	if(listen(*socketListen, SOMAXCONN) == SOCKET_ERROR) {
		KHOPANLASTERRORMESSAGE_WSA(L"listen");
		goto freeList;
	}

	LOG("[Server]: Listening socket started\n");
	PCLIENT client;

	while(TRUE) {
		SOCKADDR_IN address;
		status = sizeof(SOCKADDR_IN);
		SOCKET socket = accept(*socketListen, (struct sockaddr*) &address, &status);

		if(socket == INVALID_SOCKET) {
			if(WSAGetLastError() == WSAEINTR) break;
			KHOPANLASTERRORCONSOLE_WSA(L"accept");
			continue;
		}

		client = KHOPAN_ALLOCATE(sizeof(CLIENT));

		if(KHOPAN_ALLOCATE_FAILED(client)) {
			KHOPANERRORCONSOLE_WIN32(KHOPAN_ALLOCATE_ERROR, KHOPAN_ALLOCATE_FUNCTION);
			goto closeSocket;
		}

		if(!InetNtopW(AF_INET, &address.sin_addr, client->address, 16)) {
			KHOPANLASTERRORCONSOLE_WSA(L"InetNtopW");
			goto freeClient;
		}

		client->socket = socket;
		client->thread = CreateThread(NULL, 0, ThreadClient, client, 0, NULL);

		if(!client->thread) {
			KHOPANLASTERRORCONSOLE_WIN32(L"CreateThread");
			goto freeClient;
		}

		continue;
	freeClient:
		KHOPAN_DEALLOCATE(client);
	closeSocket:
		closesocket(socket);
	}

	if(WaitForSingleObject(clientListMutex, INFINITE) != WAIT_FAILED) {
		PLINKEDLISTITEM item;

		KHOPAN_LINKED_LIST_ITERATE(item, &clientList) {
			client = (PCLIENT) item->data;
			if(!client) continue;
			ThreadClientDisconnect(client);
			KHOPANArrayAdd(&list, (PBYTE) &client->thread);
		}

		ReleaseMutex(clientListMutex);
	}

	WaitForMultipleObjects((DWORD) list.count, (PHANDLE) list.data, TRUE, INFINITE);
	codeExit = 0;
freeList:
	KHOPANArrayFree(&list);
functionExit:
	LOG("[Server]: Exit with code: %d\n", codeExit);
	return codeExit;
}
