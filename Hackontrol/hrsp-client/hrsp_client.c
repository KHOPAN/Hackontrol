#include <WS2tcpip.h>
#include <lmcons.h>
#include <hrsp_handshake.h>
#include <hrsp_packet.h>
#include <hrsp_remote.h>
#include "hrsp_client.h"

#define ERROR_CLIENT(functionName, errorCode) if(error){error->type=HRSP_CLIENT_ERROR_TYPE_CLIENT;error->function=functionName;error->code=errorCode;}
#define ERROR_HRSP if(error){error->type=protocolError.win32?HRSP_CLIENT_ERROR_TYPE_WIN32:HRSP_CLIENT_ERROR_TYPE_HRSP;error->function=protocolError.function;error->code=protocolError.code;}
#define ERROR_WIN32(functionName, errorCode) if(error){error->type=HRSP_CLIENT_ERROR_TYPE_WIN32;error->function=functionName;error->code=errorCode;}

BOOL HRSPClientConnectToServer(const LPCSTR address, const LPCSTR port, const PHRSPCLIENTERROR error) {
	WSADATA data;
	int status = WSAStartup(MAKEWORD(2, 2), &data);

	if(status) {
		ERROR_WIN32(L"WSAStartup", status);
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
		ERROR_WIN32(L"getaddrinfo", status);
		goto cleanupResource;
	}

	SOCKET socketClient = INVALID_SOCKET;

	for(struct addrinfo* pointer = result; pointer != NULL; pointer = pointer->ai_next) {
		socketClient = socket(pointer->ai_family, pointer->ai_socktype, pointer->ai_protocol);

		if(socketClient == INVALID_SOCKET) {
			ERROR_WIN32(L"socket", WSAGetLastError());
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
		ERROR_CLIENT(L"HRSPClientConnectToServer", HRSP_CLIENT_ERROR_CANNOT_CONNECT_SERVER);
		goto cleanupResource;
	}

	HRSPDATA protocolData;
	HRSPERROR protocolError;

	if(!HRSPClientHandshake(socketClient, &protocolData, &protocolError)) {
		ERROR_HRSP;
		goto closeSocket;
	}

	DWORD size = UNLEN + 1;
	BYTE* buffer = LocalAlloc(LMEM_FIXED, size);

	if(!buffer) {
		ERROR_WIN32(L"LocalAlloc", GetLastError());
		goto closeSocket;
	}

	if(!GetUserNameA(buffer, &size)) {
		ERROR_WIN32(L"GetUserNameA", GetLastError());
		LocalFree(buffer);
		goto closeSocket;
	}

	HRSPPACKET packet;
	packet.size = size;
	packet.type = HRSP_REMOTE_CLIENT_INFORMATION_PACKET;
	packet.data = buffer;
	status = HRSPSendPacket(socketClient, &protocolData, &packet, &protocolError);
	LocalFree(buffer);

	if(!status) {
		ERROR_HRSP;
		goto closeSocket;
	}

	while(HRSPReceivePacket(socketClient, &protocolData, &packet, &protocolError)) {
		switch(packet.type) {

		}

		HRSPFreePacket(&packet, NULL);
	}

	if(!protocolError.code || (!protocolError.win32 && protocolError.code == HRSP_ERROR_CONNECTION_CLOSED) || (protocolError.win32 && protocolError.code == WSAECONNRESET)) {
		returnValue = TRUE;
		goto closeSocket;
	}

	ERROR_HRSP;
closeSocket:
	if(closesocket(socketClient) == SOCKET_ERROR) {
		ERROR_WIN32(L"closesocket", WSAGetLastError());
		returnValue = FALSE;
	}
cleanupResource:
	if(WSACleanup() == SOCKET_ERROR) {
		ERROR_WIN32(L"WSACleanup", WSAGetLastError());
		return FALSE;
	}

	return returnValue;
}
