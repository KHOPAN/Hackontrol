#include "hrsp_client.h"

#define ERROR(x,y) do{if(error){error->code=y;error->function=x;}}while(0)

BOOL HRSPConnectToServer(const LPCSTR serverAddress, const LPCSTR serverPort, const PHRSPCLIENTSTRUCT client, const PHRSPCLIENTERROR error) {
	if(!serverAddress || !serverPort || !client) {
		ERROR(L"HRSPConnectToServer", ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	WSADATA data;
	int status = WSAStartup(MAKEWORD(2, 2), &data);

	if(status) {
		ERROR(L"WSAStartup", status);
		return FALSE;
	}

	return TRUE;
}
