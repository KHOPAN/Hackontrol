#include "hrsp_protocol.h"

#define SETERROR(x) if(error){error->errorCode=1;}

BOOL HRSPClientHandshake(const SOCKET socket, const PHRSPPROTOCOLERROR error) {
	if(!socket) {
		if(error) {
			error->win32Error = FALSE;
			error->functionName = L"HRSPClientHandshake";
			error->errorCode = HRSP_ERROR_INVALID_FUNCTION_PARAMETER;
			error->win32ErrorCode = 0;
		}

		return FALSE;
	}

	return TRUE;
}

BOOL HRSPServerHandshake(const SOCKET socket, const PHRSPPROTOCOLERROR error) {
	return FALSE;
}
