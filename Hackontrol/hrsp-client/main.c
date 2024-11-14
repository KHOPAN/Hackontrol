#include "hrsp_client.h"

BOOL HRSPClientConnectToServer(const LPCWSTR address, const LPCWSTR port, const PHRSPCLIENTINPUT input, const PKHOPANERROR error) {
	return TRUE;
}

/*#include <WS2tcpip.h>
#include <libkhopan.h>
#include <lmcons.h>
#include <hrsp_handshake.h>
#include <hrsp_packet.h>
#include <hrsp_remote.h>
#include "hrsp_client_internal.h"

#define ERROR_HRSP if(error){error->type=protocolError.win32?HRSP_CLIENT_ERROR_TYPE_WIN32:HRSP_CLIENT_ERROR_TYPE_HRSP;error->code=protocolError.code;error->function=protocolError.function;}
#define ERROR_WIN32(errorCode, functionName) if(error){error->type=HRSP_CLIENT_ERROR_TYPE_WIN32;error->code=errorCode;error->function=functionName;}

static void audioCapture(const PHRSPPACKET packet, const DWORD audioThreadIdentifier) {
	if(!packet->size) {
		return;
	}

	PBYTE buffer = KHOPAN_ALLOCATE(packet->size + sizeof(WCHAR));

	if(KHOPAN_ALLOCATE_FAILED(buffer)) {
		return;
	}

	for(int i = 0; i < packet->size; i++) {
		buffer[i] = packet->data[i];
	}

	buffer[packet->size] = 0;
	buffer[packet->size + 1] = 0;
	PostThreadMessageW(audioThreadIdentifier, AM_QUERY_AUDIO_CAPTURE, 0, (LPARAM) buffer);
}

BOOL HRSPClientConnectToServer(const LPCWSTR address, const LPCWSTR port, const PHRSPCLIENTINPUT input, const PHRSPCLIENTERROR error, const LPCSTR username) {
	PHRSPCLIENTPARAMETER parameter = KHOPAN_ALLOCATE(sizeof(HRSPCLIENTPARAMETER));

	if(KHOPAN_ALLOCATE_FAILED(parameter)) {
		ERROR_WIN32(KHOPAN_ALLOCATE_ERROR, KHOPAN_ALLOCATE_FUNCTION);
		return FALSE;
	}

	for(size_t i = 0; i < sizeof(HRSPCLIENTPARAMETER); i++) {
		((PBYTE) parameter)[i] = 0;
	}

	parameter->mutex = CreateMutexExW(NULL, NULL, 0, SYNCHRONIZE | DELETE);
	BOOL codeExit = FALSE;

	if(!parameter->mutex) {
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

	parameter->socket = INVALID_SOCKET;

	for(PADDRINFOW pointer = result; pointer != NULL; pointer = pointer->ai_next) {
		parameter->socket = socket(pointer->ai_family, pointer->ai_socktype, pointer->ai_protocol);

		if(parameter->socket == INVALID_SOCKET) {
			ERROR_WIN32(WSAGetLastError(), L"socket");
			FreeAddrInfoW(result);
			goto cleanupSocket;
		}

		if(connect(parameter->socket, pointer->ai_addr, (int) pointer->ai_addrlen) != SOCKET_ERROR) {
			break;
		}

		if(closesocket(parameter->socket) == SOCKET_ERROR) {
			ERROR_WIN32(WSAGetLastError(), L"closesocket");
			goto cleanupSocket;
		}

		parameter->socket = INVALID_SOCKET;
	}

	FreeAddrInfoW(result);

	if(parameter->socket == INVALID_SOCKET) {
		if(error) {
			error->type = HRSP_CLIENT_ERROR_TYPE_CLIENT;
			error->function = L"HRSPClientConnectToServer";
			error->code = HRSP_CLIENT_ERROR_CANNOT_CONNECT_SERVER;
		}

		goto cleanupSocket;
	}

	HRSPERROR protocolError;

	if(!HRSPClientHandshake(parameter->socket, &parameter->data, &protocolError)) {
		ERROR_HRSP;
		goto closeSocket;
	}

	/*DWORD size = UNLEN + 1;
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
	//packet.size = size;
	packet.size = (int) strlen(username);
	packet.type = HRSP_REMOTE_CLIENT_INFORMATION_PACKET;
	//packet.data = buffer;
	packet.data = (PBYTE) username;
	status = HRSPSendPacket(parameter->socket, &parameter->data, &packet, &protocolError);
	//KHOPAN_DEALLOCATE(buffer);

	if(!status) {
		ERROR_HRSP;
		goto closeSocket;
	}

	if(input && input->callbackConnected) {
		input->callbackConnected(input->parameter);
	}

	parameter->stream.running = TRUE;
	HANDLE streamThread = CreateThread(NULL, 0, HRSPClientStreamThread, parameter, 0, NULL);

	if(!streamThread) {
		ERROR_WIN32(GetLastError(), L"CreateThread");
		goto closeSocket;
	}

	DWORD audioThreadIdentifier;
	HANDLE audioThread = CreateThread(NULL, 0, HRSPClientAudioThread, parameter, 0, &audioThreadIdentifier);

	if(!audioThread) {
		ERROR_WIN32(GetLastError(), L"CreateThread");
		goto closeStreamThread;
	}

	while(HRSPReceivePacket(parameter->socket, &parameter->data, &packet, &protocolError)) {
		switch(packet.type) {
		case HRSP_REMOTE_SERVER_STREAM_CODE_PACKET:
			if(WaitForSingleObject(parameter->mutex, INFINITE) == WAIT_FAILED) break;
			parameter->stream.flags = *packet.data;
			ReleaseMutex(parameter->mutex);
			break;
		case HRSP_REMOTE_SERVER_AUDIO_QUERY_DEVICE:
			PostThreadMessageW(audioThreadIdentifier, AM_QUERY_AUDIO_DEVICE, 0, 0);
			break;
		case HRSP_REMOTE_SERVER_AUDIO_CAPTURE:
			audioCapture(&packet, audioThreadIdentifier);
			break;
		}

		HRSPFreePacket(&packet, &protocolError);
	}

	if(parameter->hasError) {
		if(error) {
			*error = parameter->error;
		}

		goto closeAudioThread;
	}

	if(!protocolError.code || (!protocolError.win32 && protocolError.code == HRSP_ERROR_CONNECTION_CLOSED) || (protocolError.win32 && protocolError.code == WSAECONNRESET)) {
		codeExit = TRUE;
		goto closeAudioThread;
	}

	ERROR_HRSP;
closeAudioThread:
	PostThreadMessageW(audioThreadIdentifier, AM_EXIT, 0, 0);
	WaitForSingleObject(audioThread, INFINITE);
	CloseHandle(audioThread);
closeStreamThread:
	parameter->stream.running = FALSE;
	WaitForSingleObject(streamThread, INFINITE);
	CloseHandle(streamThread);
closeSocket:
	closesocket(parameter->socket);
cleanupSocket:
	WSACleanup();
closeMutex:
	WaitForSingleObject(parameter->mutex, INFINITE);
	CloseHandle(parameter->mutex);
streamDeallocate:
	KHOPAN_DEALLOCATE(parameter);
	return codeExit;
}*/
