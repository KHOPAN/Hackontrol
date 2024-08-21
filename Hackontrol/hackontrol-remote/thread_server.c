#include <WS2tcpip.h>
#include <khopanwin32.h>
#include "thread_server.h"
#include "thread_client.h"
#include "window_main.h"
#include "logger.h"

#include <khopanarray.h>
#include "thread_window.h"

#define REMOTE_PORT L"42485"

static SOCKET listenSocket;

DWORD WINAPI ServerThread(_In_ LPVOID parameter) {
	LOG("[Server Thread]: Hello from server thread\n");
	WSADATA socketData;
	int status = WSAStartup(MAKEWORD(2, 2), &socketData);
	int returnValue = 1;

	if(status) {
		KHWin32DialogErrorW(status, L"WSAStartup");
		goto exit;
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

		PCLIENT client = LocalAlloc(LMEM_FIXED, sizeof(CLIENT));

		if(!client) {
			KHWin32DialogErrorW(GetLastError(), L"LocalAlloc");
			goto closeSocket;
		}

		client->active = TRUE;
		client->socket = socket;
		client->stream = NULL;

		if(!InetNtopW(AF_INET, &socketAddress.sin_addr, client->address, 16)) {
			KHWin32DialogErrorW(WSAGetLastError(), L"InetNtopW");
			goto freeClient;
		}

		LOG("[Server Thread]: Client connected: %ws, starting the client thread\n" COMMA client->address);
		HANDLE thread = CreateThread(NULL, 0, ClientThread, client, CREATE_SUSPENDED, NULL);

		if(!thread) {
			KHWin32DialogErrorW(GetLastError(), L"CreateThread");
			goto freeClient;
		}

		client->windowThread = NULL;
		client->thread = thread;

		if(ResumeThread(thread) == -1) {
			KHWin32DialogErrorW(GetLastError(), L"ResumeThread");
			CloseHandle(thread);
			goto freeClient;
		}

		continue;
	freeClient:
		LocalFree(client);
	closeSocket:
		closesocket(socket);
	}

	ExitMainWindow();
	returnValue = 0;
closeListenSocket:
	closesocket(listenSocket);
cleanup:
	WSACleanup();
exit:
	LOG("[Server Thread]: Exiting the server thread (Exit code: %d)\n" COMMA returnValue);
	return returnValue;
}

void ExitServerThread() {
	if(closesocket(listenSocket) == SOCKET_ERROR) {
		int error = WSAGetLastError();

		if(error == WSANOTINITIALISED) {
			return;
		}

		KHWin32DialogErrorW(error, L"closesocket");
	}
}

HINSTANCE remoteInstance;
ArrayList clients;
HANDLE clientsLock;

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE previousInstance, _In_ LPSTR argument, _In_ int commandLine) {
	remoteInstance = instance;
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
	if(!InitializeMainWindow() || !WindowRegisterClass()) {
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

	HANDLE serverThread = CreateThread(NULL, 0, ServerThread, NULL, 0, NULL);

	if(!serverThread) {
		KHWin32DialogErrorW(GetLastError(), L"CreateThread");
		goto closeLock;
	}

	returnValue = MainWindowMessageLoop();
	ExitServerThread();
closeLock:
	CloseHandle(clientsLock);
freeClients:
	KHArrayFree(&clients);
exit:
	LOG("[Remote]: Exit with code: %d\n" COMMA returnValue);
	return returnValue;
}
