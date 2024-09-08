#include <stdio.h>
#include "hrsp_protocol.h"

#define SETERROR_HRSP(function, code) if(error){error->win32Error=FALSE;error->functionName=function;error->errorCode=code;error->win32ErrorCode=0;}
#define SETERROR_WIN32(function, code) if(error){error->win32Error=TRUE;error->functionName=function;error->errorCode=HRSP_ERROR_SUCCESS;error->win32ErrorCode=code;}

BOOL HRSPClientHandshake(const SOCKET socket, const PHRSPPROTOCOLERROR error) {
	if(!socket) {
		SETERROR_HRSP(L"HRSPClientHandshake", HRSP_ERROR_INVALID_FUNCTION_PARAMETER);
		return FALSE;
	}

	BYTE* buffer = LocalAlloc(LMEM_FIXED, 8);

	if(!buffer) {
		SETERROR_WIN32(L"LocalAlloc", GetLastError());
		return FALSE;
	}

	memcpy(buffer, "HRSP", 4);
	buffer[4] = (HRSP_PROTOCOL_VERSION >> 8) & 0xFF;
	buffer[5] = HRSP_PROTOCOL_VERSION & 0xFF;
	buffer[6] = (HRSP_PROTOCOL_VERSION_MINOR >> 8) & 0xFF;
	buffer[7] = HRSP_PROTOCOL_VERSION_MINOR & 0xFF;
	int status = send(socket, buffer, 8, 0);
	LocalFree(buffer);

	if(status == SOCKET_ERROR) {
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

	BYTE buffer[9];

	if(recv(socket, buffer, 8, 0) == SOCKET_ERROR) {
		SETERROR_WIN32(L"recv", WSAGetLastError());
		return FALSE;
	}

	buffer[8] = 0;
	printf("Buffer: %s\n", buffer);
	return TRUE;
}
