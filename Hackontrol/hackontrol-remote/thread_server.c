/*#include <WS2tcpip.h>
#include "remote.h"

#define SERVER_PORT L"42485"

extern LINKEDLIST clientList;
extern HANDLE clientListMutex;

DWORD WINAPI ThreadServer(_In_ SOCKET* socketListen) {
	if(!socketListen) {
		LOG("[Server]: Empty thread parameter\n");
		return 1;
	}

	LOG("[Server]: Initializing\n");
	ARRAYLIST list;
	KHOPANERROR error;
	DWORD codeExit = 1;

	if(!KHOPANArrayInitialize(&list, sizeof(HANDLE), &error)) {
		KHOPANERRORMESSAGE_KHOPAN(error);
		goto functionExit;
	}

	ADDRINFOW hints = {0};
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if(GetAddrInfoW(NULL, SERVER_PORT, &hints, &hints.ai_next)) {
		KHOPANLASTERRORMESSAGE_WSA(L"GetAddrInfoW");
		goto freeList;
	}

	*socketListen = WSASocketW(hints.ai_next->ai_family, hints.ai_next->ai_socktype, hints.ai_next->ai_protocol, NULL, 0, WSA_FLAG_OVERLAPPED);

	if(*socketListen == INVALID_SOCKET) {
		KHOPANLASTERRORMESSAGE_WSA(L"WSASocketW");
		FreeAddrInfoW(hints.ai_next);
		goto freeList;
	}

	if(bind(*socketListen, hints.ai_next->ai_addr, (int) hints.ai_next->ai_addrlen) == SOCKET_ERROR) {
		KHOPANLASTERRORMESSAGE_WSA(L"bind");
		FreeAddrInfoW(hints.ai_next);
		goto freeList;
	}

	FreeAddrInfoW(hints.ai_next);

	if(listen(*socketListen, SOMAXCONN) == SOCKET_ERROR) {
		KHOPANLASTERRORMESSAGE_WSA(L"listen");
		goto freeList;
	}

	LOG("[Server]: Listening socket started\n");
	SOCKADDR_IN address;
	PCLIENT client;

	while(*socketListen != INVALID_SOCKET) {
		int size = sizeof(SOCKADDR_IN);
		SOCKET socket = accept(*socketListen, (struct sockaddr*) &address, &size);

		if(socket == INVALID_SOCKET) {
			if(WSAGetLastError() == WSAEINTR) break;
			KHOPANLASTERRORCONSOLE_WSA(L"accept");
			continue;
		}

		client = KHOPAN_ALLOCATE(sizeof(CLIENT));

		if(!client) {
			KHOPANERRORCONSOLE_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"KHOPAN_ALLOCATE");
			goto closeSocket;
		}

		if(!InetNtopW(AF_INET, &address.sin_addr, client->address, sizeof(client->address) / sizeof(WCHAR))) {
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

		KHOPAN_LINKED_LIST_ITERATE_FORWARD(item, &clientList) {
			client = (PCLIENT) item->data;
			if(!client) continue;
			ThreadClientDisconnect(client);
			KHOPANArrayAdd(&list, &client->thread, NULL);
		}

		ReleaseMutex(clientListMutex);
	}

	PHANDLE handle = NULL;

	for(size_t i = 0; i < list.count; i++) {
		if(KHOPANArrayGet(&list, i, (LPVOID*) &handle, NULL)) {
			if(handle && *handle) WaitForSingleObject(*handle, INFINITE);
			handle = NULL;
		}
	}

	codeExit = 0;
freeList:
	KHOPANArrayFree(&list, NULL);
functionExit:
	WindowMainExit();
	LOG("[Server]: Exit with code: %d\n", codeExit);
	return codeExit;
}*/
