#include <WinSock2.h>
#include "hrsp.h"

#define ERROR_WSA(sourceName, functionName)               if(error){error->facility=ERROR_FACILITY_WIN32;error->code=WSAGetLastError();error->source=sourceName;error->function=functionName;}
#define ERROR_COMMON(codeError, sourceName, functionName) if(error){error->facility=ERROR_FACILITY_COMMON;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_HRSP(codeError, sourceName, functionName)   if(error){error->facility=ERROR_FACILITY_HRSP;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_CLEAR                                       ERROR_COMMON(ERROR_COMMON_SUCCESS,NULL,NULL)

LPCWSTR HRSPErrorHRSPDecoder(const PKHOPANERROR error) {
	if(!error) {
		return NULL;
	}

	if(error->facility != ERROR_FACILITY_HRSP) {
		return KHOPANErrorCommonDecoder(error);
	}

	switch(error->code) {
	case ERROR_HRSP_INVALID_MAGIC:       return L"Invalid HRSP magic number";
	case ERROR_HRSP_UNSUPPORTED_VERSION: return L"Incompatible client and server version";
	case ERROR_HRSP_CONNECTION_CLOSED:   return L"The connection was already closed";
	default:                             return L"Unknown error code";
	}
}

BOOL HRSPClientHandshake(const SOCKET socket, const PHRSPDATA data, const PKHOPANERROR error) {
	if(!socket || !data) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"HRSPClientHandshake", NULL);
		return FALSE;
	}

	BYTE buffer[8];
	buffer[0] = 'H';
	buffer[1] = 'R';
	buffer[2] = 'S';
	buffer[3] = 'P';
	buffer[4] = (HRSP_PROTOCOL_VERSION >> 8) & 0xFF;
	buffer[5] = HRSP_PROTOCOL_VERSION & 0xFF;
	buffer[6] = (HRSP_PROTOCOL_VERSION_MINOR >> 8) & 0xFF;
	buffer[7] = HRSP_PROTOCOL_VERSION_MINOR & 0xFF;

	if(send(socket, buffer, 8, 0) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPClientHandshake", L"send");
		return FALSE;
	}

	if(recv(socket, buffer, 1, MSG_WAITALL) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPClientHandshake", L"recv");
		return FALSE;
	}

	if(!buffer[0]) {
		ERROR_HRSP(ERROR_HRSP_UNSUPPORTED_VERSION, L"HRSPClientHandshake", NULL);
		return FALSE;
	}

	data->socket = socket;
	ERROR_CLEAR;
	return TRUE;
}

BOOL HRSPServerHandshake(const SOCKET socket, const PHRSPDATA data, const PKHOPANERROR error) {
	if(!socket || !data) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"HRSPServerHandshake", NULL);
		return FALSE;
	}

	BYTE buffer[8];

	if(recv(socket, buffer, 8, MSG_WAITALL) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPServerHandshake", L"recv");
		return FALSE;
	}

	if(memcmp(buffer, "HRSP", 4)) {
		ERROR_HRSP(ERROR_HRSP_INVALID_MAGIC, L"HRSPServerHandshake", NULL);
		return FALSE;
	}

	buffer[0] = ((buffer[4] << 8) | buffer[5]) == HRSP_PROTOCOL_VERSION && ((buffer[6] << 8) | buffer[7]) == HRSP_PROTOCOL_VERSION_MINOR;

	if(send(socket, buffer, 1, 0) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPServerHandshake", L"send");
		return FALSE;
	}

	if(!buffer[0]) {
		ERROR_HRSP(ERROR_HRSP_UNSUPPORTED_VERSION, L"HRSPServerHandshake", NULL);
		return FALSE;
	}

	data->socket = socket;
	ERROR_CLEAR;
	return TRUE;
}

BOOL HRSPPacketSend(const PHRSPDATA data, const PHRSPPACKET packet, const PKHOPANERROR error) {
	if(!data || !packet) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"HRSPPacketSend", NULL);
		return FALSE;
	}

	BYTE buffer[13];
	buffer[0] = packet->size > 0;
	buffer[1] = (packet->type >> 24) & 0xFF;
	buffer[2] = (packet->type >> 16) & 0xFF;
	buffer[3] = (packet->type >> 8) & 0xFF;
	buffer[4] = packet->type & 0xFF;

	if(!buffer[0]) {
		if(send(data->socket, buffer, 5, 0) == SOCKET_ERROR) {
			ERROR_WSA(L"HRSPPacketSend", L"send");
			return FALSE;
		}

		ERROR_CLEAR;
		return TRUE;
	}

	buffer[5] = (packet->size >> 56) & 0xFF;
	buffer[6] = (packet->size >> 48) & 0xFF;
	buffer[7] = (packet->size >> 40) & 0xFF;
	buffer[8] = (packet->size >> 32) & 0xFF;
	buffer[9] = (packet->size >> 24) & 0xFF;
	buffer[10] = (packet->size >> 16) & 0xFF;
	buffer[11] = (packet->size >> 8) & 0xFF;
	buffer[12] = packet->size & 0xFF;

	if(send(data->socket, buffer, sizeof(buffer), 0) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPPacketSend", L"send");
		return FALSE;
	}

	size_t pointer = 0;

	while(pointer < packet->size) {
		size_t size = packet->size - pointer;
		int sent = send(data->socket, packet->data + pointer, (int) min(size, INT_MAX), 0);

		if(sent == SOCKET_ERROR) {
			ERROR_WSA(L"HRSPPacketSend", L"send");
			return FALSE;
		}

		pointer += sent;
	}

	ERROR_CLEAR;
	return TRUE;
}

BOOL HRSPPacketReceive(const PHRSPDATA data, const PHRSPPACKET packet, const PKHOPANERROR error) {
	ERROR_CLEAR;
	return TRUE;
}

BOOL HRSPCleanup(const PHRSPDATA data, const PKHOPANERROR error) {
	ERROR_CLEAR;
	return TRUE;
}
