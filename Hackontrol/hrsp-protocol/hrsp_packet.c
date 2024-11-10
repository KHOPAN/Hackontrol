#include <WinSock2.h>
#include "hrsp_packet.h"

#define ERROR_HRSP(errorCode, functionName) if(error){error->win32=FALSE;error->code=errorCode;error->function=functionName;}
#define ERROR_WIN32(errorCode, functionName) if(error){error->win32=TRUE;error->code=errorCode;error->function=functionName;}
#define ERROR_NEUTRAL if(error){error->win32=FALSE;error->code=0;error->function=NULL;}

BOOL HRSPSendPacket(const SOCKET socket, const PHRSPDATA data, const PHRSPPACKET packet, const PKHOPANERROR error) {
	/*if(!socket || !data || !packet) {
		ERROR_HRSP(HRSP_ERROR_INVALID_FUNCTION_PARAMETER, L"HRSPSendPacket");
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
		ERROR_WIN32(WSAGetLastError(), L"send");
		return FALSE;
	}

	if(size && send(socket, packet->data, packet->size, 0) == SOCKET_ERROR) {
		ERROR_WIN32(WSAGetLastError(), L"send");
		return FALSE;
	}

	ERROR_NEUTRAL;*/
	return TRUE;
}

BOOL HRSPReceivePacket(const SOCKET socket, const PHRSPDATA data, const PHRSPPACKET packet, const PKHOPANERROR error) {
	/*if(!socket || !data || !packet) {
		ERROR_HRSP(HRSP_ERROR_INVALID_FUNCTION_PARAMETER, L"HRSPReceivePacket");
		return FALSE;
	}

	BYTE header[8];
	int result = recv(socket, header, sizeof(header), MSG_WAITALL);

	if(result == SOCKET_ERROR) {
		ERROR_WIN32(WSAGetLastError(), L"recv");
		return FALSE;
	}

	if(!result) {
		ERROR_HRSP(HRSP_ERROR_CONNECTION_CLOSED, L"HRSPReceivePacket");
		return FALSE;
	}

	int size = ((header[0] & 0xFF) << 24) | ((header[1] & 0xFF) << 16) | ((header[2] & 0xFF) << 8) | (header[3] & 0xFF);
	UINT type = ((header[4] & 0xFF) << 24) | ((header[5] & 0xFF) << 16) | ((header[6] & 0xFF) << 8) | (header[7] & 0xFF);

	if(size < 1) {
		packet->size = 0;
		packet->type = type;
		packet->data = NULL;
		ERROR_NEUTRAL;
		return TRUE;
	}

	PBYTE buffer = LocalAlloc(LMEM_FIXED, size);

	if(!buffer) {
		ERROR_WIN32(GetLastError(), L"LocalAlloc");
		return FALSE;
	}

	result = recv(socket, buffer, size, MSG_WAITALL);

	if(result == SOCKET_ERROR) {
		ERROR_WIN32(WSAGetLastError(), L"recv");
		LocalFree(buffer);
		return FALSE;
	}

	if(!result) {
		ERROR_HRSP(HRSP_ERROR_CONNECTION_CLOSED, L"HRSPReceivePacket");
		LocalFree(buffer);
		return FALSE;
	}

	packet->size = size;
	packet->type = type;
	packet->data = buffer;
	ERROR_NEUTRAL;*/
	return TRUE;
}

BOOL HRSPFreePacket(const PHRSPPACKET packet, const PKHOPANERROR error) {
	/*if(!packet) {
		ERROR_HRSP(HRSP_ERROR_INVALID_FUNCTION_PARAMETER, L"HRSPFreePacket");
		return FALSE;
	}

	if(!packet->data || packet->size < 1) {
		ERROR_NEUTRAL;
		return TRUE;
	}

	if(LocalFree(packet->data)) {
		ERROR_WIN32(GetLastError(), L"LocalFree");
		return FALSE;
	}

	packet->size = 0;
	packet->type = 0;
	packet->data = NULL;
	ERROR_NEUTRAL;*/
	return TRUE;
}

BOOL HRSPSendTypePacket(const SOCKET socket, const PHRSPDATA data, const UINT type, const PKHOPANERROR error) {
	/*HRSPPACKET packet = {0};
	packet.type = type;
	return HRSPSendPacket(socket, data, &packet, error);*/
	return TRUE;
}
