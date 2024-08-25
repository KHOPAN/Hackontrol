#include <WS2tcpip.h>
#include "hrsp_client.h"

#define REMOTE_ERROR(x,y) do{if(error){error->code=y;error->function=x;}}while(0)

BOOL HRSPConnectToServer(const LPCSTR serverAddress, const LPCSTR serverPort, const PHRSPCLIENTSTRUCT client, const PHRSPCLIENTERROR error) {
	if(!serverAddress || !serverPort || !client) {
		REMOTE_ERROR(L"HRSPConnectToServer", ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	WSADATA data;
	int status = WSAStartup(MAKEWORD(2, 2), &data);

	if(status) {
		REMOTE_ERROR(L"WSAStartup", status);
		return FALSE;
	}

	struct addrinfo hints = {0};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	struct addrinfo* result;
	status = getaddrinfo(serverAddress, serverPort, &hints, &result);
	BOOL returnValue = FALSE;

	if(status) {
		REMOTE_ERROR(L"getaddrinfo", status);
		goto cleanup;
	}

	SOCKET socketClient = INVALID_SOCKET;

	for(struct addrinfo* pointer = result; pointer != NULL; pointer = pointer->ai_next) {
		socketClient = socket(pointer->ai_family, pointer->ai_socktype, pointer->ai_protocol);

		if(socketClient == INVALID_SOCKET) {
			REMOTE_ERROR(L"socket", WSAGetLastError());
			freeaddrinfo(result);
			goto cleanup;
		}

		status = connect(socketClient, pointer->ai_addr, (int) pointer->ai_addrlen);

		if(status != SOCKET_ERROR) {
			break;
		}

		closesocket(socketClient);
		socketClient = INVALID_SOCKET;
	}

	freeaddrinfo(result);

	if(socketClient == INVALID_SOCKET) {
		REMOTE_ERROR(L"HRSPConnectToServer", ERROR_CONNECTION_REFUSED);
		goto cleanup;
	}

	returnValue = TRUE;
cleanup:
	if(WSACleanup() == SOCKET_ERROR) {
		REMOTE_ERROR(L"WSACleanup", WSAGetLastError());
		return FALSE;
	}

	return returnValue;
}
