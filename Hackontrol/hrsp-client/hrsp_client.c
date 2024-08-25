#include "hrsp_client.h"

void HRSPConnectToServer(const LPCSTR serverAddress, const LPCSTR serverPort, const PHRSPCLIENTSTRUCT client) {
	if(!client) {
		return;
	}

	if(!serverAddress || !serverPort) {
		client->error.present = TRUE;
		client->error.code = ERROR_INVALID_PARAMETER;
		client->error.function = L"HRSPConnectToServer";
		return;
	}

	WSADATA data;
	int status = WSAStartup(MAKEWORD(2, 2), &data);

	if(status) {
		client->error.present = TRUE;
		client->error.code = status;
		client->error.function = L"WSAStartup";
		return;
	}

	client->error.present = FALSE;
}
