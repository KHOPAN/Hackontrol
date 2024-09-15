#include <WS2tcpip.h>
#include <lmcons.h>
#include <hrsp_handshake.h>
#include <hrsp_packet.h>
#include <hrsp_remote.h>
#include "hrsp_client_internal.h"

#define ERROR_CLIENT(errorCode) if(error){error->type=HRSP_CLIENT_ERROR_TYPE_CLIENT;error->function=L"HRSPClientConnectToServer";error->code=errorCode;}
#define ERROR_HRSP if(error){error->type=protocolError.win32?HRSP_CLIENT_ERROR_TYPE_WIN32:HRSP_CLIENT_ERROR_TYPE_HRSP;error->function=protocolError.function;error->code=protocolError.code;}
#define ERROR_WIN32(functionName, errorCode) if(error){error->type=HRSP_CLIENT_ERROR_TYPE_WIN32;error->function=functionName;error->code=errorCode;}

BOOL HRSPClientConnectToServer(const LPCWSTR address, const LPCWSTR port, const PHRSPCLIENTINPUT input, const PHRSPCLIENTERROR error) {
	if(!input) {
		ERROR_CLIENT(HRSP_CLIENT_ERROR_INVALID_FUNCTION_PARAMETER);
		return FALSE;
	}

	PHRSPCLIENTSTREAMPARAMETER stream = LocalAlloc(LMEM_FIXED, sizeof(HRSPCLIENTSTREAMPARAMETER));

	if(!stream) {
		ERROR_WIN32(L"LocalAlloc", GetLastError());
		return FALSE;
	}

	for(size_t i = 0; i < sizeof(HRSPCLIENTSTREAMPARAMETER); i++) {
		((PBYTE) stream)[i] = 0;
	}

	stream->mutex = CreateMutexExW(NULL, NULL, 0, SYNCHRONIZE | DELETE);
	BOOL returnValue = FALSE;

	if(!stream->mutex) {
		ERROR_WIN32(L"CreateMutexExW", GetLastError());
		goto freeStream;
	}

	WSADATA data;
	int status = WSAStartup(MAKEWORD(2, 2), &data);

	if(status) {
		ERROR_WIN32(L"WSAStartup", status);
		goto closeMutex;
	}

	ADDRINFOW hints = {0};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	ADDRINFOW* result;
	status = GetAddrInfoW(address ? address : L"localhost", port ? port : HRSP_PROTOCOL_PORT_STRING, &hints, &result);

	if(status) {
		ERROR_WIN32(L"getaddrinfo", status);
		goto cleanupSocket;
	}

	stream->socket = INVALID_SOCKET;

	for(PADDRINFOW pointer = result; pointer != NULL; pointer = pointer->ai_next) {
		stream->socket = socket(pointer->ai_family, pointer->ai_socktype, pointer->ai_protocol);

		if(stream->socket == INVALID_SOCKET) {
			ERROR_WIN32(L"socket", WSAGetLastError());
			FreeAddrInfoW(result);
			goto cleanupSocket;
		}

		if(connect(stream->socket, pointer->ai_addr, (int) pointer->ai_addrlen) != SOCKET_ERROR) {
			break;
		}

		if(closesocket(stream->socket) == SOCKET_ERROR) {
			ERROR_WIN32(L"closesocket", WSAGetLastError());
			goto cleanupSocket;
		}

		stream->socket = INVALID_SOCKET;
	}

	FreeAddrInfoW(result);

	if(stream->socket == INVALID_SOCKET) {
		ERROR_CLIENT(HRSP_CLIENT_ERROR_CANNOT_CONNECT_SERVER);
		goto cleanupSocket;
	}

	HRSPERROR protocolError;

	if(!HRSPClientHandshake(stream->socket, &stream->data, &protocolError)) {
		ERROR_HRSP;
		goto closeSocket;
	}

	DWORD size = UNLEN + 1;
	PBYTE buffer = LocalAlloc(LMEM_FIXED, size);

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
	status = HRSPSendPacket(stream->socket, &stream->data, &packet, &protocolError);
	LocalFree(buffer);

	if(!status) {
		ERROR_HRSP;
		goto closeSocket;
	}

	stream->running = TRUE;
	HANDLE streamThread = CreateThread(NULL, 0, HRSPClientStreamThread, stream, 0, NULL);

	if(!streamThread) {
		ERROR_WIN32(L"CreateThread", GetLastError());
		goto closeSocket;
	}

	while(HRSPReceivePacket(stream->socket, &stream->data, &packet, &protocolError)) {
		if(packet.type != HRSP_REMOTE_SERVER_STREAM_CODE_PACKET) {
			HRSPFreePacket(&packet, &protocolError);
			continue;
		}

		if(WaitForSingleObject(stream->mutex, INFINITE) == WAIT_FAILED) {
			ERROR_WIN32(L"WaitForSingleObject", GetLastError());
			goto closeStreamThread;
		}

		stream->flags = *packet.data;
		ReleaseMutex(stream->mutex);
		HRSPFreePacket(&packet, &protocolError);
	}

	if(stream->hasError) {
		if(error) {
			(*error) = stream->error;
		}

		goto closeStreamThread;
	}

	if(!protocolError.code || (!protocolError.win32 && protocolError.code == HRSP_ERROR_CONNECTION_CLOSED) || (protocolError.win32 && protocolError.code == WSAECONNRESET)) {
		returnValue = TRUE;
		goto closeStreamThread;
	}

	ERROR_HRSP;
closeStreamThread:
	stream->running = FALSE;

	if(WaitForSingleObject(streamThread, INFINITE) == WAIT_FAILED) {
		ERROR_WIN32(L"WaitForSingleObject", GetLastError());
		returnValue = FALSE;
	}

	CloseHandle(streamThread);
closeSocket:
	if(closesocket(stream->socket) == SOCKET_ERROR && (status = WSAGetLastError()) != WSAENOTSOCK) {
		ERROR_WIN32(L"closesocket", status);
		returnValue = FALSE;
	}
cleanupSocket:
	if(WSACleanup() == SOCKET_ERROR) {
		ERROR_WIN32(L"WSACleanup", WSAGetLastError());
		returnValue = FALSE;
	}
closeMutex:
	CloseHandle(stream->mutex);
freeStream:
	LocalFree(stream);
	return returnValue;
}
