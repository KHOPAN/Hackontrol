#include "hrsp_protocol.h"

#define SETERROR_HRSP(functionName, errorCode) if(error){error->win32=FALSE;error->function=functionName;error->code=errorCode;}
#define SETERROR_WIN32(functionName, errorCode) if(error){error->win32=TRUE;error->function=functionName;error->code=errorCode;}

BOOL HRSPSendPacket(const SOCKET socket, const PHRSPPROTOCOLDATA data, const PHRSPPROTOCOLPACKET packet, const PHRSPPROTOCOLERROR error) {
	if(!socket || !data || !packet) {
		SETERROR_HRSP(L"HRSPSendPacket", HRSP_ERROR_INVALID_FUNCTION_PARAMETER);
		return FALSE;
	}

	return TRUE;
}

BOOL HRSPReceivePacket(const SOCKET socket, const PHRSPPROTOCOLDATA data, const PHRSPPROTOCOLPACKET packet, const PHRSPPROTOCOLERROR error) {
	if(!socket || !data || !packet) {
		SETERROR_HRSP(L"HRSPReceivePacket", HRSP_ERROR_INVALID_FUNCTION_PARAMETER);
		return FALSE;
	}

	return TRUE;
}
