#include <WS2tcpip.h>
#include <hrsp_protocol.h>
#include "hrsp_client.h"

#define SETERROR_CLIENT(functionName, errorCode) if(error){error->type=HRSP_CLIENT_ERROR_TYPE_CLIENT;error->function=functionName;error->code=errorCode;}
#define SETERROR_WIN32(functionName, errorCode) if(error){error->type=HRSP_CLIENT_ERROR_TYPE_WIN32;error->function=functionName;error->code=errorCode;}

BOOL HRSPClientConnectToServer(const LPCSTR address, const LPCSTR port, const PHRSPCLIENTERROR error) {
	WSADATA data;
	int status = WSAStartup(MAKEWORD(2, 2), &data);

	if(status) {
		SETERROR_WIN32(L"WSAStartup", status);
		return FALSE;
	}

	struct addrinfo hints = {0};
	struct addrinfo* result;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	status = getaddrinfo(address ? address : "localhost", port ? port : HRSP_PROTOCOL_PORT_STRING, &hints, &result);
	BOOL returnValue = FALSE;

	if(status) {
		SETERROR_WIN32(L"getaddrinfo", status);
		goto cleanupResource;
	}

	SOCKET socketClient = INVALID_SOCKET;

	for(struct addrinfo* pointer = result; pointer != NULL; pointer = pointer->ai_next) {
		socketClient = socket(pointer->ai_family, pointer->ai_socktype, pointer->ai_protocol);

		if(socketClient == INVALID_SOCKET) {
			SETERROR_WIN32(L"socket", WSAGetLastError());
			freeaddrinfo(result);
			goto cleanupResource;
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
		SETERROR_CLIENT(L"HRSPClientConnectToServer", HRSP_CLIENT_ERROR_CANNOT_CONNECT_SERVER);
		goto cleanupResource;
	}

	HRSPPROTOCOLDATA protocolData;
	HRSPPROTOCOLERROR protocolError;

	if(!HRSPClientHandshake(socketClient, &protocolData, &protocolError)) {
		if(error) {
			error->type = protocolError.win32 ? HRSP_CLIENT_ERROR_TYPE_WIN32 : HRSP_CLIENT_ERROR_TYPE_HRSP;
			error->function = protocolError.function;
			error->code = protocolError.code;
		}

		goto closeSocket;
	}

	returnValue = TRUE;
closeSocket:
	if(closesocket(socketClient) == SOCKET_ERROR) {
		SETERROR_WIN32(L"closesocket", WSAGetLastError());
		returnValue = FALSE;
	}
cleanupResource:
	if(WSACleanup() == SOCKET_ERROR) {
		SETERROR_WIN32(L"WSACleanup", WSAGetLastError());
		return FALSE;
	}

	return returnValue;
}
