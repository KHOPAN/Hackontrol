#include <stdio.h>
#include "hrsp_protocol.h"

#pragma warning(disable: 6385)

#define SETERROR_HRSP(function, code) if(error){error->win32Error=FALSE;error->functionName=function;error->errorCode=code;error->win32ErrorCode=0;}
#define SETERROR_WIN32(function, code) if(error){error->win32Error=TRUE;error->functionName=function;error->errorCode=HRSP_ERROR_SUCCESS;error->win32ErrorCode=code;}

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

	WORD version = (buffer[4] << 8) | buffer[5];
	WORD versionMinor = (buffer[6] << 8) | buffer[7];

	if(version != HRSP_PROTOCOL_VERSION || versionMinor != HRSP_PROTOCOL_VERSION_MINOR) {
		SETERROR_HRSP(L"recv", version > HRSP_PROTOCOL_VERSION ? HRSP_ERROR_UNSUPPORTED_VERSION_HIGHER : version < HRSP_PROTOCOL_VERSION ? HRSP_ERROR_UNSUPPORTED_VERSION_LOWER : versionMinor > HRSP_PROTOCOL_VERSION_MINOR ? HRSP_ERROR_UNSUPPORTED_VERSION_HIGHER : HRSP_ERROR_UNSUPPORTED_VERSION_LOWER);
		return FALSE;
	}

	return TRUE;
}
