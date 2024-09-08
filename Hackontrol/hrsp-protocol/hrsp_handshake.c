#include "hrsp_protocol.h"

#define SETERROR_HRSP(functionName, errorCode) if(error){error->win32=FALSE;error->function=functionName;error->code=errorCode;}
#define SETERROR_WIN32(functionName, errorCode) if(error){error->win32=TRUE;error->function=functionName;error->code=errorCode;}

#pragma warning(disable: 6385)

BOOL HRSPClientHandshake(const SOCKET socket, const PHRSPPROTOCOLERROR error) {
	if(!socket) {
		SETERROR_HRSP(L"HRSPClientHandshake", HRSP_ERROR_INVALID_FUNCTION_PARAMETER);
		return FALSE;
	}

	BYTE buffer[8];
	memcpy(buffer, "HRSP", 4);
	buffer[4] = (HRSP_PROTOCOL_VERSION >> 8) & 0xFF;
	buffer[5] = HRSP_PROTOCOL_VERSION & 0xFF;
	buffer[6] = (HRSP_PROTOCOL_VERSION_MINOR >> 8) & 0xFF;
	buffer[7] = HRSP_PROTOCOL_VERSION_MINOR & 0xFF;

	if(send(socket, buffer, 8, 0) == SOCKET_ERROR) {
		SETERROR_WIN32(L"send", WSAGetLastError());
		return FALSE;
	}

	if(recv(socket, buffer, 1, 0) == SOCKET_ERROR) {
		SETERROR_WIN32(L"recv", WSAGetLastError());
		return FALSE;
	}

	if(buffer[0] != HRSP_ERROR_SUCCESS) {
		switch(buffer[0]) {
		case HRSP_ERROR_UNSUPPORTED_VERSION: SETERROR_HRSP(L"HRSPClientHandshake", HRSP_ERROR_UNSUPPORTED_VERSION);
		default:                             SETERROR_HRSP(L"HRSPClientHandshake", HRSP_ERROR_UNKNOWN_ERROR);
		}

		return FALSE;
	}

	return TRUE;
}

BOOL HRSPServerHandshake(const SOCKET socket, const PHRSPPROTOCOLERROR error) {
	if(!socket) {
		SETERROR_HRSP(L"HRSPServerHandshake", HRSP_ERROR_INVALID_FUNCTION_PARAMETER);
		return FALSE;
	}

	BYTE buffer[8];

	if(recv(socket, buffer, 8, 0) == SOCKET_ERROR) {
		SETERROR_WIN32(L"recv", WSAGetLastError());
		return FALSE;
	}

	if(memcmp(buffer, "HRSP", 4)) {
		SETERROR_HRSP(L"recv", HRSP_ERROR_INVALID_MAGIC);
		return FALSE;
	}

	buffer[1] = ((buffer[4] << 8) | buffer[5]) != HRSP_PROTOCOL_VERSION || ((buffer[6] << 8) | buffer[7]) != HRSP_PROTOCOL_VERSION_MINOR;
	buffer[0] = buffer[1] ? HRSP_ERROR_UNSUPPORTED_VERSION : HRSP_ERROR_SUCCESS;

	if(send(socket, buffer, 1, 0) == SOCKET_ERROR) {
		SETERROR_WIN32(L"send", WSAGetLastError());
		return FALSE;
	}

	if(buffer[1]) {
		SETERROR_HRSP(L"HRSPServerHandshake", HRSP_ERROR_UNSUPPORTED_VERSION);
		return FALSE;
	}

	return TRUE;
}
