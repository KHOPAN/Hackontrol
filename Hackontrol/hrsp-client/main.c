#include <WS2tcpip.h>
#include <libkhopan.h>
#include <lmcons.h>
#include <hrsp_handshake.h>
#include <hrsp_packet.h>
#include <hrsp_remote.h>
#include "hrsp_client_internal.h"

#define ERROR_HRSP if(error){error->type=protocolError.win32?HRSP_CLIENT_ERROR_TYPE_WIN32:HRSP_CLIENT_ERROR_TYPE_HRSP;error->code=protocolError.code;error->function=protocolError.function;}
#define ERROR_WIN32(errorCode, functionName) if(error){error->type=HRSP_CLIENT_ERROR_TYPE_WIN32;error->code=errorCode;error->function=functionName;}

BOOL HRSPClientConnectToServer(const LPCWSTR address, const LPCWSTR port, const PHRSPCLIENTINPUT input, const PHRSPCLIENTERROR error) {
	PHRSPCLIENTSTREAMPARAMETER stream = KHOPAN_ALLOCATE(sizeof(HRSPCLIENTSTREAMPARAMETER));

	if(KHOPAN_ALLOCATE_FAILED(stream)) {
		ERROR_WIN32(KHOPAN_ALLOCATE_ERROR, KHOPAN_ALLOCATE_FUNCTION);
		return FALSE;
	}

	for(size_t i = 0; i < sizeof(HRSPCLIENTSTREAMPARAMETER); i++) {
		((PBYTE) stream)[i] = 0;
	}

	stream->mutex = CreateMutexExW(NULL, NULL, 0, SYNCHRONIZE | DELETE);
	BOOL codeExit = FALSE;

	if(!stream->mutex) {
		ERROR_WIN32(GetLastError(), L"CreateMutexExW");
		goto streamDeallocate;
	}

	WSADATA data;
	int status = WSAStartup(MAKEWORD(2, 2), &data);

	if(status) {
		ERROR_WIN32(status, L"WSAStartup");
		goto closeMutex;
	}

	ADDRINFOW hints = {0};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	PADDRINFOW result;
	status = GetAddrInfoW(address ? address : L"localhost", port ? port : HRSP_PROTOCOL_PORT_STRING, &hints, &result);

	if(status) {
		ERROR_WIN32(status, L"GetAddrInfoW");
		goto cleanupSocket;
	}

	stream->socket = INVALID_SOCKET;

	for(PADDRINFOW pointer = result; pointer != NULL; pointer = pointer->ai_next) {
		stream->socket = socket(pointer->ai_family, pointer->ai_socktype, pointer->ai_protocol);

		if(stream->socket == INVALID_SOCKET) {
			ERROR_WIN32(WSAGetLastError(), L"socket");
			FreeAddrInfoW(result);
			goto cleanupSocket;
		}

		if(connect(stream->socket, pointer->ai_addr, (int) pointer->ai_addrlen) != SOCKET_ERROR) {
			break;
		}

		if(closesocket(stream->socket) == SOCKET_ERROR) {
			ERROR_WIN32(WSAGetLastError(), L"closesocket");
			goto cleanupSocket;
		}

		stream->socket = INVALID_SOCKET;
	}

	FreeAddrInfoW(result);

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
	PBYTE buffer = KHOPAN_ALLOCATE(size);

	if(KHOPAN_ALLOCATE_FAILED(buffer)) {
		ERROR_WIN32(KHOPAN_ALLOCATE_ERROR, KHOPAN_ALLOCATE_FUNCTION);
		goto closeSocket;
	}

	if(!GetUserNameA(buffer, &size)) {
		ERROR_WIN32(GetLastError(), L"GetUserNameA");
		KHOPAN_DEALLOCATE(buffer);
		goto closeSocket;
	}

	HRSPPACKET packet;
	packet.size = size;
	packet.type = HRSP_REMOTE_CLIENT_INFORMATION_PACKET;
	packet.data = buffer;
	status = HRSPSendPacket(stream->socket, &stream->data, &packet, &protocolError);
	KHOPAN_DEALLOCATE(buffer);

	if(!status) {
		ERROR_HRSP;
		goto closeSocket;
	}

	if(input && input->callbackConnected) {
		input->callbackConnected(input->parameter);
	}

	stream->running = TRUE;
	HANDLE streamThread = CreateThread(NULL, 0, HRSPClientStreamThread, stream, 0, NULL);

	if(!streamThread) {
		ERROR_WIN32(GetLastError(), L"CreateThread");
		goto closeSocket;
	}

	while(HRSPReceivePacket(stream->socket, &stream->data, &packet, &protocolError)) {
		switch(packet.type) {
		case HRSP_REMOTE_SERVER_STREAM_CODE_PACKET:
			if(WaitForSingleObject(stream->mutex, INFINITE) == WAIT_FAILED) break;
			stream->flags = *packet.data;
			ReleaseMutex(stream->mutex);
			break;
		}

		HRSPFreePacket(&packet, &protocolError);
	}

	if(stream->hasError) {
		if(error) {
			(*error) = stream->error;
		}

		goto closeStreamThread;
	}

	if(!protocolError.code || (!protocolError.win32 && protocolError.code == HRSP_ERROR_CONNECTION_CLOSED) || (protocolError.win32 && protocolError.code == WSAECONNRESET)) {
		codeExit = TRUE;
		goto closeStreamThread;
	}

	ERROR_HRSP;
closeStreamThread:
	stream->running = FALSE;
	WaitForSingleObject(streamThread, INFINITE);
	CloseHandle(streamThread);
closeSocket:
	closesocket(stream->socket);
cleanupSocket:
	WSACleanup();
closeMutex:
	WaitForSingleObject(stream->mutex, INFINITE);
	CloseHandle(stream->mutex);
streamDeallocate:
	KHOPAN_DEALLOCATE(stream);
	return codeExit;
}
