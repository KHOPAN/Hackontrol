#include <WS2tcpip.h>
#include <khopanwin32.h>
#include "thread_server.h"
#include "logger.h"

#define REMOTE_PORT L"42485"

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
	SOCKET listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	FreeAddrInfoW(result);

	if(listenSocket == INVALID_SOCKET) {
		KHWin32DialogErrorW(WSAGetLastError(), L"socket");
		goto cleanup;
	}
cleanup:
	WSACleanup();
	return 0;
}
