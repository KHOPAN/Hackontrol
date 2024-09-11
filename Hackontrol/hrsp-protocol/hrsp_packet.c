#include "hrsp_packet.h"

#define ERROR_HRSP(functionName, errorCode) if(error){error->win32=FALSE;error->function=functionName;error->code=errorCode;}
#define ERROR_WIN32(functionName, errorCode) if(error){error->win32=TRUE;error->function=functionName;error->code=errorCode;}

BOOL HRSPSendPacket(const SOCKET socket, const PHRSPDATA data, const PHRSPPACKET packet, const PHRSPERROR error) {
	if(!socket || !data || !packet) {
		ERROR_HRSP(L"HRSPSendPacket", HRSP_ERROR_INVALID_FUNCTION_PARAMETER);
		return FALSE;
	}

	return TRUE;
}

BOOL HRSPReceivePacket(const SOCKET socket, const PHRSPDATA data, const PHRSPPACKET packet, const PHRSPERROR error) {
	if(!socket || !data || !packet) {
		ERROR_HRSP(L"HRSPReceivePacket", HRSP_ERROR_INVALID_FUNCTION_PARAMETER);
		return FALSE;
	}

	return TRUE;
}
