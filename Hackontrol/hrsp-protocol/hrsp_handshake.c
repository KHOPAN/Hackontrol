#include "hrsp_protocol.h"

#define SETERROR_HRSP(function, code) if(error){error->win32Error=FALSE;error->functionName=function;error->errorCode=code;error->win32ErrorCode=0;}

BOOL HRSPClientHandshake(const SOCKET socket, const PHRSPPROTOCOLERROR error) {
	if(!socket) {
		SETERROR_HRSP(L"HRSPClientHandshake", HRSP_ERROR_INVALID_FUNCTION_PARAMETER);
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
