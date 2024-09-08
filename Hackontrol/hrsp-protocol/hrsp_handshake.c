#include "hrsp_protocol.h"

#define SETERROR_HRSP(function, code) if(error){error->win32Error=FALSE;error->functionName=function;error->errorCode=code;error->win32ErrorCode=0;}
#define SETERROR_WIN32(function, code) if(error){error->win32Error=TRUE;error->functionName=function;error->errorCode=HRSP_ERROR_SUCCESS;error->win32ErrorCode=code;}

BOOL HRSPClientHandshake(const SOCKET socket, const PHRSPPROTOCOLERROR error) {
	if(!socket) {
		SETERROR_HRSP(L"HRSPClientHandshake", HRSP_ERROR_INVALID_FUNCTION_PARAMETER);
		return FALSE;
	}

	if(send(socket, NULL, 0, 0) == SOCKET_ERROR) {
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

	return FALSE;
}
