#include <WinSock2.h>
#include "hrsp_packet.h"

#define ERROR_HRSP(functionName, errorCode) if(error){error->win32=FALSE;error->function=functionName;error->code=errorCode;}
#define ERROR_WIN32(functionName, errorCode) if(error){error->win32=TRUE;error->function=functionName;error->code=errorCode;}

BOOL HRSPSendPacket(const SOCKET socket, const PHRSPDATA data, const PHRSPPACKET packet, const PHRSPERROR error) {
	if(!socket || !data || !packet) {
		ERROR_HRSP(L"HRSPSendPacket", HRSP_ERROR_INVALID_FUNCTION_PARAMETER);
		return FALSE;
	}

	int size = packet->size < 1 ? 0 : packet->size;
	BYTE header[8];
	header[0] = (size >> 24) & 0xFF;
	header[1] = (size >> 16) & 0xFF;
	header[2] = (size >> 8) & 0xFF;
	header[3] = size & 0xFF;
	header[4] = (packet->type >> 24) & 0xFF;
	header[5] = (packet->type >> 16) & 0xFF;
	header[6] = (packet->type >> 8) & 0xFF;
	header[7] = packet->type & 0xFF;

	if(send(socket, header, sizeof(header), 0) == SOCKET_ERROR) {
		ERROR_WIN32(L"send", WSAGetLastError());
		return FALSE;
	}

	if(size && send(socket, packet->data, packet->size, 0) == SOCKET_ERROR) {
		ERROR_WIN32(L"send", WSAGetLastError());
		return FALSE;
	}

	return TRUE;
}

BOOL HRSPReceivePacket(const SOCKET socket, const PHRSPDATA data, const PHRSPPACKET packet, const PHRSPERROR error) {
	if(!socket || !data || !packet) {
		ERROR_HRSP(L"HRSPReceivePacket", HRSP_ERROR_INVALID_FUNCTION_PARAMETER);
		return FALSE;
	}

	BYTE header[8];

	if(recv(socket, header, sizeof(header), MSG_WAITALL) == SOCKET_ERROR) {
		ERROR_WIN32(L"recv", WSAGetLastError());
		return FALSE;
	}

	int size = ((header[0] & 0xFF) << 24) | ((header[1] & 0xFF) << 16) | ((header[2] & 0xFF) << 8) | (header[3] & 0xFF);
	unsigned int type = ((header[4] & 0xFF) << 24) | ((header[5] & 0xFF) << 16) | ((header[6] & 0xFF) << 8) | (header[7] & 0xFF);

	if(size < 1) {
		packet->size = 0;
		packet->type = type;
		packet->data = NULL;
		return TRUE;
	}

	BYTE* buffer = LocalAlloc(LMEM_FIXED, size);

	if(!buffer) {
		ERROR_WIN32(L"LocalAlloc", GetLastError());
		return FALSE;
	}

	if(recv(socket, buffer, size, MSG_WAITALL) == SOCKET_ERROR) {
		ERROR_WIN32(L"recv", WSAGetLastError());
		LocalFree(buffer);
		return FALSE;
	}

	packet->size = size;
	packet->type = type;
	packet->data = buffer;
	return TRUE;
}

BOOL HRSPFreePacket(const PHRSPPACKET packet, const PHRSPERROR error) {
	if(!packet) {
		ERROR_HRSP(L"HRSPFreePacket", HRSP_ERROR_INVALID_FUNCTION_PARAMETER);
		return FALSE;
	}

	if(!packet->data || packet->size < 1) {
		return TRUE;
	}

	if(LocalFree(packet->data)) {
		ERROR_WIN32(L"LocalFree", GetLastError());
		return FALSE;
	}

	packet->size = 0;
	packet->type = 0;
	packet->data = NULL;
	return TRUE;
}
