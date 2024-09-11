#include "hrsp_packet.h"

#define ERROR_HRSP(functionName, errorCode) if(error){error->win32=FALSE;error->function=functionName;error->code=errorCode;}
#define ERROR_WIN32(functionName, errorCode) if(error){error->win32=TRUE;error->function=functionName;error->code=errorCode;}

BOOL HRSPSendPacket(const SOCKET socket, const PHRSPDATA data, const PHRSPPACKET packet, const PHRSPERROR error) {
	if(!socket || !data || !packet) {
		ERROR_HRSP(L"HRSPSendPacket", HRSP_ERROR_INVALID_FUNCTION_PARAMETER);
		return FALSE;
	}

	char header[8];
	header[0] = (packet->size >> 24) & 0xFF;
	header[1] = (packet->size >> 16) & 0xFF;
	header[2] = (packet->size >> 8) & 0xFF;
	header[3] = packet->size & 0xFF;
	header[4] = (packet->type >> 24) & 0xFF;
	header[5] = (packet->type >> 16) & 0xFF;
	header[6] = (packet->type >> 8) & 0xFF;
	header[7] = packet->type & 0xFF;

	if(!send(socket, header, 8, 0)) {
		ERROR_WIN32(L"send", WSAGetLastError());
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
