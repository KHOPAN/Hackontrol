#include <WS2tcpip.h>
#include <hrsp_protocol.h>
#include "hrsp_client.h"

#define SETERROR_CLIENT(functionName, errorCode) if(error){error->type=HRSP_CLIENT_ERROR_TYPE_CLIENT;error->function=functionName;error->code=errorCode;}
#define SETERROR_HRSP(functionName, errorCode) if(error){error->type=HRSP_CLIENT_ERROR_TYPE_HRSP;error->function=functionName;error->code=errorCode;}
#define SETERROR_WIN32(functionName, errorCode) if(error){error->type=HRSP_CLIENT_ERROR_TYPE_WIN32;error->function=functionName;error->code=errorCode;}

BOOL HRSPClientConnectToServer(const LPCSTR address, const LPCSTR port, const PHRSPCLIENTERROR error) {
	WSADATA data;
	int status = WSAStartup(MAKEWORD(2, 2), &data);

	if(status) {
		SETERROR_WIN32(L"WSAStartup", status);
		return FALSE;
	}
cleanupResource:
	if(WSACleanup() == SOCKET_ERROR) {
		SETERROR_WIN32(L"WSACleanup", WSAGetLastError());
		return FALSE;
	}

	return TRUE;
}

/*#define REMOTE_ERROR(x,y) do{if(error){error->remoteError=FALSE;error->function=x;error->code=y;}}while(0)
#define REMOTE_CUSTOM_ERROR(x,y) do{if(error){error->remoteError=TRUE;error->function=x;error->codeRemote=y;}}while(0)

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
		REMOTE_CUSTOM_ERROR(L"HRSPConnectToServer", REMOTE_ERROR_UNABLE_TO_CONNECT_TO_SERVER);
		goto cleanup;
	}

	const char* header = "HRSP 1.0 CONNECT";
	status = send(socketClient, header, (int) strlen(header), 0);

	if(status == SOCKET_ERROR) {
		REMOTE_ERROR(L"send", WSAGetLastError());
		goto closeSocket;
	}

	char buffer[12];
	status = recv(socketClient, buffer, 11, 0);

	if(status == SOCKET_ERROR) {
		REMOTE_ERROR(L"recv", WSAGetLastError());
		goto closeSocket;
	}

	buffer[11] = 0;

	if(strcmp(buffer, "HRSP 1.0 OK")) {
		REMOTE_CUSTOM_ERROR(L"HRSPConnectToServer", REMOTE_ERROR_SERVER_SEND_INVALID_RESPOSE);
		goto closeSocket;
	}

	HRSPPROTOCOLERROR protocolError;

	if(!HRSPClientHandshake(socketClient, &protocolError)) {
		REMOTE_ERROR(L"HRSPClientHandshake", protocolError.win32ErrorCode);
		goto closeSocket;
	}

	returnValue = TRUE;
closeSocket:
	if(closesocket(socketClient) == SOCKET_ERROR) {
		REMOTE_ERROR(L"closesocket", WSAGetLastError());
		returnValue = FALSE;
	}
cleanup:
	if(WSACleanup() == SOCKET_ERROR) {
		REMOTE_ERROR(L"WSACleanup", WSAGetLastError());
		return FALSE;
	}

	return returnValue;
}*/
