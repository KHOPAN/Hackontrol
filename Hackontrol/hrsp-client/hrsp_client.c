#include "hrsp_client.h"

BOOL HRSPConnectToServer(const LPCSTR serverAddress, const LPCSTR serverPort, const PHRSPCLIENTSTRUCT client) {
	if(!client) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	WSADATA data;
	int status = WSAStartup(MAKEWORD(2, 2), &data);

	if(status) {
		SetLastError(status);
		return FALSE;
	}

	return TRUE;
}
