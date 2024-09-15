#include <WS2tcpip.h>
#include <lmcons.h>
#include <hrsp_handshake.h>
#include <hrsp_packet.h>
#include <hrsp_remote.h>
#include "hrsp_client_internal.h"

#define ERROR_HRSP if(error){error->type=protocolError.win32?HRSP_CLIENT_ERROR_TYPE_WIN32:HRSP_CLIENT_ERROR_TYPE_HRSP;error->function=protocolError.function;error->code=protocolError.code;}
#define ERROR_WIN32(functionName, errorCode) if(error){error->type=HRSP_CLIENT_ERROR_TYPE_WIN32;error->function=functionName;error->code=errorCode;}

BOOL HRSPClientConnectToServer(const LPCSTR address, const LPCSTR port, const PHRSPCLIENTERROR error) {
	PHRSPCLIENTSTREAMPARAMETER stream = LocalAlloc(LMEM_FIXED, sizeof(HRSPCLIENTSTREAMPARAMETER));

	if(!stream) {
		ERROR_WIN32(L"LocalAlloc", GetLastError());
		return FALSE;
	}

	for(size_t i = 0; i < sizeof(HRSPCLIENTSTREAMPARAMETER); i++) {
		((PBYTE) stream)[i] = 0;
	}

	stream->sensitive.mutex = CreateMutexExW(NULL, NULL, 0, SYNCHRONIZE | DELETE);
	BOOL returnValue = FALSE;

	if(!stream->sensitive.mutex) {
		ERROR_WIN32(L"CreateMutexExW", GetLastError());
		goto freeStream;
	}

	WSADATA data;
	int status = WSAStartup(MAKEWORD(2, 2), &data);

	if(status) {
		ERROR_WIN32(L"WSAStartup", status);
		goto closeSensitiveMutex;
	}

	ADDRINFOA hints = {0};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	ADDRINFOA* result;
	status = getaddrinfo(address ? address : "localhost", port ? port : HRSP_PROTOCOL_PORT_STRING, &hints, &result);

	if(status) {
		ERROR_WIN32(L"getaddrinfo", status);
		goto cleanupSocket;
	}

	stream->socket = INVALID_SOCKET;

	for(PADDRINFOA pointer = result; pointer != NULL; pointer = pointer->ai_next) {
		stream->socket = socket(pointer->ai_family, pointer->ai_socktype, pointer->ai_protocol);

		if(stream->socket == INVALID_SOCKET) {
			ERROR_WIN32(L"socket", WSAGetLastError());
			freeaddrinfo(result);
			goto cleanupSocket;
		}

		if(connect(stream->socket, pointer->ai_addr, (int) pointer->ai_addrlen) != SOCKET_ERROR) break;

		if(closesocket(stream->socket) == SOCKET_ERROR) {
			ERROR_WIN32(L"closesocket", WSAGetLastError());
			goto cleanupSocket;
		}

		stream->socket = INVALID_SOCKET;
	}

	freeaddrinfo(result);

	if(stream->socket == INVALID_SOCKET) {
		if(error) {
			error->type = HRSP_CLIENT_ERROR_TYPE_CLIENT;
			error->function = L"HRSPClientConnectToServer";
			error->code = HRSP_CLIENT_ERROR_CANNOT_CONNECT_SERVER;
		}

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

		if(LocalFree(buffer)) {
			ERROR_WIN32(L"LocalFree", GetLastError());
		}

		goto closeSocket;
	}

	HRSPPACKET packet;
	packet.size = size;
	packet.type = HRSP_REMOTE_CLIENT_INFORMATION_PACKET;
	packet.data = buffer;
	status = HRSPSendPacket(stream->socket, &stream->data, &packet, &protocolError);

	if(LocalFree(buffer)) {
		ERROR_WIN32(L"LocalFree", GetLastError());
		goto closeSocket;
	}

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
			goto freePacket;
		}

		if(WaitForSingleObject(stream->sensitive.mutex, INFINITE) == WAIT_FAILED) {
			ERROR_WIN32(L"WaitForSingleObject", GetLastError());
			goto closeStreamThread;
		}

		stream->sensitive.flags = *packet.data;

		if(!ReleaseMutex(stream->sensitive.mutex)) {
			ERROR_WIN32(L"ReleaseMutex", GetLastError());
			goto closeStreamThread;
		}
	freePacket:
		if(!HRSPFreePacket(&packet, &protocolError)) {
			ERROR_HRSP;
			goto closeStreamThread;
		}
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

	if(!CloseHandle(streamThread)) {
		ERROR_WIN32(L"CloseHandle", GetLastError());
		returnValue = FALSE;
	}
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
closeSensitiveMutex:
	if(!CloseHandle(stream->sensitive.mutex)) {
		ERROR_WIN32(L"CloseHandle", GetLastError());
		returnValue = FALSE;
	}
freeStream:
	if(LocalFree(stream)) {
		ERROR_WIN32(L"LocalFree", GetLastError());
		return FALSE;
	}

	return returnValue;
}
