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
	DWORD returnValue = 1;

	if(status) {
		KHWin32DialogErrorW(status, L"WSAStartup");
		goto exit;
	}

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

	LOG("[Server]: Start the listening socket\n");
	socketListen = WSASocketW(result->ai_family, result->ai_socktype, result->ai_protocol, NULL, 0, WSA_FLAG_OVERLAPPED);

	if(socketListen == INVALID_SOCKET) {
		KHWin32DialogErrorW(WSAGetLastError(), L"WSASocketW");
		FreeAddrInfoW(result);
		goto cleanup;
	}

	status = bind(socketListen, result->ai_addr, (int) result->ai_addrlen);
	FreeAddrInfoW(result);

	if(status == SOCKET_ERROR) {
		KHWin32DialogErrorW(WSAGetLastError(), L"bind");
		closesocket(socketListen);
		socketListen = 0;
		goto cleanup;
	}

	if(listen(socketListen, SOMAXCONN) == SOCKET_ERROR) {
		KHWin32DialogErrorW(WSAGetLastError(), L"listen");
		closesocket(socketListen);
		socketListen = 0;
		goto cleanup;
	}

	LOG("[Server]: Listening for incoming connection...\n");
	SOCKADDR_IN address;

	while(TRUE) {
		status = sizeof(SOCKADDR_IN);
		SOCKET socket = accept(socketListen, (struct sockaddr*) &address, &status);

		if(socket == INVALID_SOCKET) {
			status = WSAGetLastError();

			if(status == WSAEINTR) {
				break;
			}

			KHWin32DialogErrorW(status, L"accept");
			continue;
		}

		PCLIENT client = LocalAlloc(LMEM_FIXED, sizeof(CLIENT));

		if(!client) {
			KHWin32DialogErrorW(GetLastError(), L"LocalAlloc");
			goto closeSocket;
		}

		client->active = TRUE;
		client->socket = socket;
		client->name = NULL;
		client->window = NULL;

		if(!InetNtopW(AF_INET, &address.sin_addr, client->address, 16)) {
			KHWin32DialogErrorW(WSAGetLastError(), L"InetNtopW");
			goto freeClient;
		}

		LOG("[Server]: Client connected: %ws\n" COMMA client->address);
		client->thread = CreateThread(NULL, 0, ClientThread, client, 0, NULL);

		if(!client->thread) {
			KHWin32DialogErrorW(GetLastError(), L"CreateThread");
			goto freeClient;
		}

		continue;
	freeClient:
		LocalFree(client);
	closeSocket:
		closesocket(socket);
	}

	MainWindowExit();
	returnValue = 0;
cleanup:
	if(socketListen) {
		closesocket(socketListen);
		socketListen = 0;
	}

	WSACleanup();
exit:
	LOG("[Server]: Exit server with code: %d\n" COMMA returnValue);
	return returnValue;
}

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE previousInstance, _In_ LPSTR argument, _In_ int commandLine) {
	int returnValue = 1;
#ifdef LOGGER_ENABLE
#ifndef NO_CONSOLE
	if(!AllocConsole()) {
		KHWin32DialogErrorW(GetLastError(), L"AllocConsole");
		goto exit;
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
		goto exit;
	}

	if(!KHArrayInitialize(&clients, sizeof(CLIENT))) {
		KHWin32DialogErrorW(GetLastError(), L"KHArrayInitialize");
		goto exit;
	}

	clientsLock = CreateMutexExW(NULL, NULL, 0, SYNCHRONIZE | DELETE);

	if(!clientsLock) {
		KHWin32DialogErrorW(GetLastError(), L"CreateMutexExW");
		goto freeClients;
	}

	HANDLE serverThreadHandle = CreateThread(NULL, 0, serverThread, NULL, 0, NULL);

	if(!serverThreadHandle) {
		KHWin32DialogErrorW(GetLastError(), L"CreateThread");
		goto closeLock;
	}

	returnValue = MainWindowMessageLoop();
	LOG("[Remote]: Wait for server thread to exit\n");

	if(socketListen) {
		closesocket(socketListen);
		socketListen = 0;
	}

	if(WaitForSingleObject(serverThreadHandle, INFINITE) == WAIT_FAILED) {
		KHWin32DialogErrorW(GetLastError(), L"WaitForSingleObject");
	}

	LOG("[Remote]: Wait for client list mutex to unlock\n");

	if(WaitForSingleObject(clientsLock, INFINITE) == WAIT_FAILED) {
		KHWin32DialogErrorW(GetLastError(), L"WaitForSingleObject");
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

		if(client->thread) {
			WaitForSingleObject(client->thread, INFINITE);
		}
	}
closeServer:
	CloseHandle(serverThreadHandle);
closeLock:
	CloseHandle(clientsLock);
freeClients:
	KHArrayFree(&clients);
exit:
	LOG("[Remote]: Exit with code: %d\n" COMMA returnValue);
	Sleep(INFINITE);
	return returnValue;
}
