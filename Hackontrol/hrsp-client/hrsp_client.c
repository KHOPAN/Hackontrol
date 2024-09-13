#include <WS2tcpip.h>
#include <lmcons.h>
#include <hrsp_handshake.h>
#include <hrsp_packet.h>
#include <hrsp_remote.h>
#include "hrsp_client.h"
#include "hrsp_client_internal.h"

#define ERROR_CLIENT(functionName, errorCode) if(error){error->type=HRSP_CLIENT_ERROR_TYPE_CLIENT;error->function=functionName;error->code=errorCode;}
#define ERROR_HRSP if(error){error->type=protocolError.win32?HRSP_CLIENT_ERROR_TYPE_WIN32:HRSP_CLIENT_ERROR_TYPE_HRSP;error->function=protocolError.function;error->code=protocolError.code;}
#define ERROR_WIN32(functionName, errorCode) if(error){error->type=HRSP_CLIENT_ERROR_TYPE_WIN32;error->function=functionName;error->code=errorCode;}

BOOL clientHRSPIsRunning;
SOCKET clientHRSPSocket;
PHRSPDATA clientHRSPData;
BYTE clientHRSPStreamFlags;

BOOL HRSPClientConnectToServer(const LPCSTR address, const LPCSTR port, const PHRSPCLIENTERROR error) {
	clientHRSPIsRunning = TRUE;
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

	clientHRSPSocket = INVALID_SOCKET;

	for(struct addrinfo* pointer = result; pointer != NULL; pointer = pointer->ai_next) {
		clientHRSPSocket = socket(pointer->ai_family, pointer->ai_socktype, pointer->ai_protocol);

		if(clientHRSPSocket == INVALID_SOCKET) {
			ERROR_WIN32(L"socket", WSAGetLastError());
			freeaddrinfo(result);
			goto cleanupResource;
		}

		status = connect(clientHRSPSocket, pointer->ai_addr, (int) pointer->ai_addrlen);

		if(status != SOCKET_ERROR) {
			break;
		}

		closesocket(clientHRSPSocket);
		clientHRSPSocket = INVALID_SOCKET;
	}

	freeaddrinfo(result);

	if(clientHRSPSocket == INVALID_SOCKET) {
		ERROR_CLIENT(L"HRSPClientConnectToServer", HRSP_CLIENT_ERROR_CANNOT_CONNECT_SERVER);
		goto cleanupResource;
	}

	clientHRSPData = LocalAlloc(LMEM_FIXED, sizeof(HRSPDATA));

	if(!clientHRSPData) {
		ERROR_WIN32(L"LocalAlloc", GetLastError());
		goto closeSocket;
	}

	HRSPERROR protocolError;

	if(!HRSPClientHandshake(clientHRSPSocket, clientHRSPData, &protocolError)) {
		ERROR_HRSP;
		goto freeHRSPData;
	}

	DWORD size = UNLEN + 1;
	PBYTE buffer = LocalAlloc(LMEM_FIXED, size);

	if(!buffer) {
		ERROR_WIN32(L"LocalAlloc", GetLastError());
		goto freeHRSPData;
	}

	if(!GetUserNameA(buffer, &size)) {
		ERROR_WIN32(L"GetUserNameA", GetLastError());
		LocalFree(buffer);
		goto freeHRSPData;
	}

	HRSPPACKET packet;
	packet.size = size;
	packet.type = HRSP_REMOTE_CLIENT_INFORMATION_PACKET;
	packet.data = buffer;
	status = HRSPSendPacket(clientHRSPSocket, clientHRSPData, &packet, &protocolError);
	LocalFree(buffer);

	if(!status) {
		ERROR_HRSP;
		goto freeHRSPData;
	}

	HANDLE streamThread = CreateThread(NULL, 0, HRSPClientStreamThread, NULL, 0, NULL);

	if(!streamThread) {
		ERROR_WIN32(L"CreateThread", GetLastError());
		goto freeHRSPData;
	}

	while(HRSPReceivePacket(clientHRSPSocket, clientHRSPData, &packet, &protocolError)) {
		switch(packet.type) {
		case HRSP_REMOTE_SERVER_STREAM_CODE_PACKET:
			clientHRSPStreamFlags = *packet.data;
			break;
		}

		HRSPFreePacket(&packet, NULL);
	}

	if(!protocolError.code || (!protocolError.win32 && protocolError.code == HRSP_ERROR_CONNECTION_CLOSED) || (protocolError.win32 && protocolError.code == WSAECONNRESET)) {
		returnValue = TRUE;
		goto closeStreamThread;
	}

	ERROR_HRSP;
closeStreamThread:
	clientHRSPIsRunning = FALSE;

	if(WaitForSingleObject(streamThread, INFINITE) == WAIT_FAILED) {
		ERROR_WIN32(L"WaitForSingleObject", GetLastError());
		returnValue = FALSE;
	}

	CloseHandle(streamThread);
freeHRSPData:
	LocalFree(clientHRSPData);
closeSocket:
	if(closesocket(clientHRSPSocket) == SOCKET_ERROR) {
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
