#include "hrsp_client.h"

#define REMOTE_ERROR(x,y) do{if(error){error->code=y;error->function=x;}}while(0)

BOOL HRSPConnectToServer(const LPCSTR serverAddress, const LPCSTR serverPort, const PHRSPCLIENTSTRUCT client, const PHRSPCLIENTERROR error) {
	if(!serverAddress || !serverPort || !client) {
		REMOTE_ERROR(L"HRSPConnectToServer", ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	WSADATA data;
	int status = WSAStartup(MAKEWORD(2, 2), &data);

	if(status) {
		REMOTE_ERROR(L"WSAStartup", status);
		return FALSE;
	}

	if(WSACleanup() == SOCKET_ERROR) {
		REMOTE_ERROR(L"WSACleanup", WSAGetLastError());
		return FALSE;
	}

	return TRUE;
}
