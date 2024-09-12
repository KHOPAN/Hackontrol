#include <WS2tcpip.h>
#include <khopanwin32.h>
#include <khopanarray.h>
#include "logger.h"
#include "thread_window.h"
#include "window_main.h"

#define REMOTE_PORT L"42485"

ArrayList clients;
HANDLE clientsLock;

static SOCKET socketListen;

DWORD WINAPI serverThread(_In_ LPVOID parameter) {
	LOG("[Server]: Starting\n");
	WSADATA data;
	int status = WSAStartup(MAKEWORD(2, 2), &data);
	data.wVersion = 1;

	if(status) {
		KHWIN32_ERROR(status, L"WSAStartup");
		goto functionExit;
	}

	ADDRINFOW hints = {0};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	status = GetAddrInfoW(NULL, REMOTE_PORT, &hints, &hints.ai_next);

	if(status) {
		KHWIN32_ERROR(status, L"GetAddrInfoW");
		goto cleanupResources;
	}

	LOG("[Server]: Start the listening socket\n");
	socketListen = WSASocketW(hints.ai_next->ai_family, hints.ai_next->ai_socktype, hints.ai_next->ai_protocol, NULL, 0, WSA_FLAG_OVERLAPPED);

	if(socketListen == INVALID_SOCKET) {
		KHWIN32_LAST_WSA_ERROR(L"WSASocketW");
		FreeAddrInfoW(hints.ai_next);
		goto cleanupResources;
	}

	status = bind(socketListen, hints.ai_next->ai_addr, (int) hints.ai_next->ai_addrlen);
	FreeAddrInfoW(hints.ai_next);

	if(status == SOCKET_ERROR) {
		KHWIN32_LAST_WSA_ERROR(L"bind");
		goto cleanupResources;
	}

	if(listen(socketListen, SOMAXCONN) == SOCKET_ERROR) {
		KHWIN32_LAST_WSA_ERROR(L"listen");
		goto cleanupResources;
	}

	LOG("[Server]: Listening for incoming connection...\n");

	while(TRUE) {
		SOCKADDR_IN address;
		status = sizeof(SOCKADDR_IN);
		SOCKET socket = accept(socketListen, (struct sockaddr*) &address, &status);

		if(socket == INVALID_SOCKET) {
			status = WSAGetLastError();
			if(status == WSAEINTR) break;
			KHWIN32_ERROR(status, L"accept");
			continue;
		}

		PCLIENT client = LocalAlloc(LMEM_FIXED, sizeof(CLIENT));

		if(!client) {
			KHWIN32_LAST_ERROR(L"LocalAlloc");
			goto closeSocket;
		}

		client->active = TRUE;
		client->connected = FALSE;
		client->socket = socket;
		client->name = NULL;
		client->window = NULL;

		if(!InetNtopW(AF_INET, &address.sin_addr, client->address, 16)) {
			KHWIN32_LAST_WSA_ERROR(L"InetNtopW");
			goto freeClient;
		}

		LOG("[Server]: Client connected: %ws\n" COMMA client->address);
		client->thread = CreateThread(NULL, 0, ClientThread, client, 0, NULL);

		if(!client->thread) {
			KHWIN32_LAST_ERROR(L"CreateThread");
			goto freeClient;
		}

		continue;
	freeClient:
		LocalFree(client);
	closeSocket:
		closesocket(socket);
	}

	MainWindowExit();
	data.wVersion = 0;
cleanupResources:
	if(socketListen) {
		closesocket(socketListen);
		socketListen = 0;
	}

	if(WSACleanup() == SOCKET_ERROR) {
		KHWIN32_LAST_WSA_ERROR(L"WSACleanup");
		data.wVersion = 1;
	}
functionExit:
	LOG("[Server]: Exit server with code: %d\n" COMMA data.wVersion);
	return data.wVersion;
}

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE previousInstance, _In_ LPSTR argument, _In_ int commandLine) {
	int returnValue = 1;
#ifdef LOGGER_ENABLE
#ifndef NO_CONSOLE
	if(!AllocConsole()) {
		KHWIN32_LAST_ERROR(L"AllocConsole");
		goto functionExit;
	}

	FILE* file = stdout;
	freopen_s(&file, "CONOUT$", "w", stdout);
	file = stderr;
	freopen_s(&file, "CONOUT$", "w", stderr);
	SetWindowTextW(GetConsoleWindow(), L"Remote Log");
#endif
#endif
	LOG("[Remote]: Starting\n");

	if(!MainWindowInitialize(instance) || !ClientWindowInitialize(instance)) {
		goto functionExit;
	}

	if(!KHArrayInitialize(&clients, sizeof(CLIENT))) {
		KHWIN32_LAST_ERROR(L"KHArrayInitialize");
		goto functionExit;
	}

	clientsLock = CreateMutexExW(NULL, NULL, 0, SYNCHRONIZE | DELETE);

	if(!clientsLock) {
		KHWIN32_LAST_ERROR(L"CreateMutexExW");
		goto freeClients;
	}

	HANDLE serverThreadHandle = CreateThread(NULL, 0, serverThread, NULL, 0, NULL);

	if(!serverThreadHandle) {
		KHWIN32_LAST_ERROR(L"CreateThread");
		goto closeClientsLock;
	}

	returnValue = MainWindowMessageLoop();
	LOG("[Remote]: Wait for server thread to exit\n");

	if(socketListen) {
		closesocket(socketListen);
		socketListen = 0;
	}

	if(WaitForSingleObject(serverThreadHandle, INFINITE) == WAIT_FAILED) {
		KHWIN32_LAST_ERROR(L"WaitForSingleObject");
		returnValue = 1;
		goto closeServerThreadHandle;
	}

	LOG("[Remote]: Wait for client list mutex to unlock\n");

	if(WaitForSingleObject(clientsLock, INFINITE) == WAIT_FAILED) {
		KHWIN32_LAST_ERROR(L"WaitForSingleObject");
		returnValue = 1;
		goto closeServerThreadHandle;
	}

	LOG("[Remote]: Wait for all client threads to exit\n");

	for(size_t i = 0; i < clients.elementCount; i++) {
		PCLIENT client = (PCLIENT) clients.data + clients.elementSize * i;

		if(!client->active) {
			continue;
		}

		if(client->socket) {
			closesocket(client->socket);
			client->socket = 0;
		}

		if(!client->thread) {
			continue;
		}

		if(WaitForSingleObject(client->thread, INFINITE) == WAIT_FAILED) {
			KHWIN32_LAST_ERROR(L"WaitForSingleObject");
			returnValue = 1;
			goto closeServerThreadHandle;
		}
	}
closeServerThreadHandle:
	CloseHandle(serverThreadHandle);
closeClientsLock:
	CloseHandle(clientsLock);
freeClients:
	KHArrayFree(&clients);
functionExit:
	LOG("[Remote]: Exit with code: %d\n" COMMA returnValue);
#ifdef LOGGER_ENABLE
	Sleep(INFINITE);
#endif
	return returnValue;
}
