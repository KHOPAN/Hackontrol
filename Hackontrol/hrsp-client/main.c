#include <WS2tcpip.h>
#include <lmcons.h>
#include <hrsp_remote.h>
#include "hrsp_client.h"

#define ERROR_WIN32(codeError, sourceName, functionName)  if(error){error->facility=ERROR_FACILITY_WIN32;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_WSA(sourceName, functionName)               if(error){error->facility=ERROR_FACILITY_WIN32;error->code=WSAGetLastError();error->source=sourceName;error->function=functionName;}
#define ERROR_COMMON(codeError, sourceName, functionName) if(error){error->facility=ERROR_FACILITY_COMMON;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_CLEAR                                       ERROR_COMMON(ERROR_COMMON_SUCCESS,NULL,NULL)
#define ERROR_SOURCE(sourceName)                          if(error){error->function=error->source;error->source=sourceName;}

BOOL HRSPClientConnectToServer(const LPCWSTR address, const LPCWSTR port, const PHRSPCLIENTINPUT input, const PKHOPANERROR error) {
	WSADATA data;
	int status = WSAStartup(MAKEWORD(2, 2), &data);

	if(status) {
		ERROR_WIN32(status, L"HRSPClientConnectToServer", L"WSAStartup");
		return 1;
	}

	ADDRINFOW hints = {0};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	BOOL codeExit = FALSE;

	if(GetAddrInfoW(address ? address : L"localhost", port ? port : HRSP_PROTOCOL_PORT_STRING, &hints, &hints.ai_next)) {
		ERROR_WSA(L"HRSPClientConnectToServer", L"GetAddrInfoW");
		goto cleanupSocket;
	}

	SOCKET clientSocket = INVALID_SOCKET;

	for(PADDRINFOW pointer = hints.ai_next; pointer != NULL; pointer = pointer->ai_next) {
		clientSocket = socket(pointer->ai_family, pointer->ai_socktype, pointer->ai_protocol);

		if(clientSocket == INVALID_SOCKET) {
			ERROR_WSA(L"HRSPClientConnectToServer", L"socket");
			FreeAddrInfoW(hints.ai_next);
			goto cleanupSocket;
		}

		if(connect(clientSocket, pointer->ai_addr, (int) pointer->ai_addrlen) != SOCKET_ERROR) {
			break;
		}

		closesocket(clientSocket);
		clientSocket = INVALID_SOCKET;
	}

	FreeAddrInfoW(hints.ai_next);

	if(clientSocket == INVALID_SOCKET) {
		ERROR_COMMON(ERROR_COMMON_FUNCTION_FAILED, L"HRSPClientConnectToServer", NULL);
		goto cleanupSocket;
	}

	HRSPDATA protocolData;

	if(!HRSPClientInitialize(clientSocket, &protocolData, error)) {
		ERROR_SOURCE(L"HRSPClientConnectToServer");
		goto closeSocket;
	}

	DWORD size = UNLEN + 1;
	PBYTE buffer = KHOPAN_ALLOCATE(size * sizeof(WCHAR));

	if(!buffer) {
		ERROR_COMMON(ERROR_COMMON_FUNCTION_FAILED, L"HRSPClientConnectToServer", L"KHOPAN_ALLOCATE");
		goto cleanupProtocol;
	}

	if(!GetUserNameW((LPWSTR) buffer, &size)) {
		ERROR_WIN32(GetLastError(), L"HRSPClientConnectToServer", L"GetUserNameW");
		KHOPAN_DEALLOCATE(buffer);
		goto cleanupProtocol;
	}

	HRSPPACKET packet;
	packet.type = HRSP_REMOTE_CLIENT_USERNAME;
	packet.size = size * sizeof(WCHAR);
	packet.data = buffer;
	size = HRSPPacketSend(&protocolData, &packet, error);
	KHOPAN_DEALLOCATE(buffer);

	if(!size) {
		ERROR_SOURCE(L"HRSPClientConnectToServer");
		goto cleanupProtocol;
	}

	if(input && input->callbackConnected) {
		input->callbackConnected(input->parameter);
	}

	while(HRSPPacketReceive(&protocolData, &packet, error)) {
		switch(packet.type) {
		case HRSP_REMOTE_SERVER_STREAM_REQUEST:
			printf("Stream Request\n");
			break;
		}

		if(packet.data) {
			printf("Packet: %.*s\n", (unsigned int) packet.size, (LPCSTR) packet.data);
			KHOPAN_DEALLOCATE(packet.data);
		}
	}

	if(error->facility != ERROR_FACILITY_HRSP || error->code != ERROR_HRSP_CONNECTION_CLOSED) {
		ERROR_SOURCE(L"HRSPClientConnectToServer");
		goto cleanupProtocol;
	}

	codeExit = TRUE;
	ERROR_CLEAR;
cleanupProtocol:
	HRSPClientCleanup(&protocolData);
closeSocket:
	closesocket(clientSocket);
cleanupSocket:
	WSACleanup();
	return codeExit;
}

/*#include <WS2tcpip.h>
#include <libkhopan.h>
#include <hrsp_handshake.h>
#include <hrsp_packet.h>
#include "hrsp_client_internal.h"

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
