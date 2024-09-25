#include <WinSock2.h>
#include "hrsp_handshake.h"

#define ERROR_HRSP(errorCode, functionName) if(error){error->win32=FALSE;error->code=errorCode;error->function=functionName;}
#define ERROR_WIN32(errorCode, functionName) if(error){error->win32=TRUE;error->code=errorCode;error->function=functionName;}
#define ERROR_NEUTRAL if(error){error->win32=FALSE;error->code=0;error->function=NULL;}

BOOL HRSPClientHandshake(const SOCKET socket, const PHRSPDATA data, const PHRSPERROR error) {
	if(!socket || !data) {
		ERROR_HRSP(HRSP_ERROR_INVALID_FUNCTION_PARAMETER, L"HRSPClientHandshake");
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
		ERROR_WIN32(WSAGetLastError(), L"send");
		return FALSE;
	}

	if(recv(socket, buffer, 1, MSG_WAITALL) == SOCKET_ERROR) {
		ERROR_WIN32(WSAGetLastError(), L"recv");
		return FALSE;
	}

	if(buffer[0] != HRSP_ERROR_SUCCESS) {
		ERROR_HRSP(buffer[0] == HRSP_ERROR_UNSUPPORTED_VERSION ? HRSP_ERROR_UNSUPPORTED_VERSION : HRSP_ERROR_UNKNOWN_ERROR, L"HRSPClientHandshake");
		return FALSE;
	}

	ERROR_NEUTRAL;
	return TRUE;
}

BOOL HRSPServerHandshake(const SOCKET socket, const PHRSPDATA data, const PHRSPERROR error) {
	if(!socket || !data) {
		ERROR_HRSP(HRSP_ERROR_INVALID_FUNCTION_PARAMETER, L"HRSPServerHandshake");
		return FALSE;
	}

	BYTE buffer[8];

	if(recv(socket, buffer, 8, MSG_WAITALL) == SOCKET_ERROR) {
		ERROR_WIN32(WSAGetLastError(), L"recv");
		return FALSE;
	}

	if(memcmp(buffer, "HRSP", 4)) {
		ERROR_HRSP(HRSP_ERROR_INVALID_MAGIC, L"recv");
		return FALSE;
	}

	BOOL versionMatch = ((buffer[4] << 8) | buffer[5]) == HRSP_PROTOCOL_VERSION && ((buffer[6] << 8) | buffer[7]) == HRSP_PROTOCOL_VERSION_MINOR;
	buffer[0] = versionMatch ? HRSP_ERROR_SUCCESS : HRSP_ERROR_UNSUPPORTED_VERSION;

	if(send(socket, buffer, 1, 0) == SOCKET_ERROR) {
		ERROR_WIN32(WSAGetLastError(), L"send");
		return FALSE;
	}

	if(!versionMatch) {
		ERROR_HRSP(HRSP_ERROR_UNSUPPORTED_VERSION, L"HRSPServerHandshake");
		return FALSE;
	}

	ERROR_NEUTRAL;
	return TRUE;
}
