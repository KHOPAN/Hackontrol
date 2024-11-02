#include <WinSock2.h>
#include <hrsp_handshake.h>
#include <hrsp_remote.h>
#include "remote.h"

#ifdef LOGGER_ENABLE
#define ERROR_HRSP(function) do{LPWSTR internal_message=HRSPGetErrorMessage(function,&protocolError);if(internal_message){LOG("[Client %ws]: %ws",client->address,internal_message);LocalFree(internal_message);}}while(0)
#else
#define ERROR_HRSP(function)
#endif

extern LINKEDLIST clientList;
extern HANDLE clientListMutex;

DWORD WINAPI ThreadClient(_In_ PCLIENT client) {
	if(!client) {
		LOG("[Client]: Empty thread parameter\n");
		return 1;
	}

	LOG("[Client %ws]: Initializing\n", client->address);
	HRSPERROR protocolError;
	DWORD codeExit = 1;
	PLINKEDLISTITEM item = NULL;
	BOOL freeClient = FALSE;

	if(!HRSPServerHandshake(client->socket, &client->hrsp, &protocolError)) {
		ERROR_HRSP(L"HRSPServerHandshake");
		goto functionExit;
	}

	HRSPPACKET packet;

	if(!HRSPReceivePacket(client->socket, &client->hrsp, &packet, &protocolError)) {
		ERROR_HRSP(L"HRSPReceivePacket");
		goto functionExit;
	}

	if(packet.type != HRSP_REMOTE_CLIENT_INFORMATION_PACKET) {
		LOG("[Client %ws]: Invalid first packet type: %u\n", client->address, packet.type);
		HRSPFreePacket(&packet, NULL);
		goto functionExit;
	}

	client->name = KHOPANFormatMessage(L"%S", packet.data);
	HRSPFreePacket(&packet, NULL);
	LOG("[Client %ws]: Username: '%ws'\n", client->address, client->name);

	if(!WindowMainAdd(&client, &item)) {
		freeClient = TRUE;
		goto freeName;
	}

	while(HRSPReceivePacket(client->socket, &client->hrsp, &packet, &protocolError)) {
		if(!WindowSessionHandlePacket(client, &packet)) {
			LOG("[Client %ws]: Unknown packet type: %u\n", client->address, packet.type);
		}

		HRSPFreePacket(&packet, NULL);
	}

	if(!protocolError.code || (!protocolError.win32 && protocolError.code == HRSP_ERROR_CONNECTION_CLOSED) || (protocolError.win32 && (protocolError.code == WSAEINTR || protocolError.code == WSAECONNABORTED || protocolError.code == WSAECONNRESET || protocolError.code == WSAENOTSOCK))) {
		codeExit = 0;
		goto closeSession;
	}

	ERROR_HRSP(L"HRSPReceivePacket");
closeSession:
	if(client->session.thread) {
		WindowSessionClose(client);
		WaitForSingleObject(client->session.thread, INFINITE);
	}
freeName:
	if(client->name) {
		LocalFree(client->name);
	}
functionExit:
	LOG("[Client %ws]: Exit with code: %d\n", client->address, codeExit);
	closesocket(client->socket);
	CloseHandle(client->thread);

	if(freeClient) {
		KHOPAN_DEALLOCATE(client);
	} else if(item && !WindowMainRemove(item)) {
		codeExit = 1;
	}

	return codeExit;
}

void ThreadClientOpen(const PCLIENT client) {
	if(client->session.thread) {
		WindowSessionClose(client);
		WaitForSingleObject(client->session.thread, INFINITE);
	}

	client->session.thread = CreateThread(NULL, 0, WindowSession, client, 0, NULL);

	if(!client->session.thread) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateThread");
	}
}

void ThreadClientDisconnect(const PCLIENT client) {
	if(client->socket != INVALID_SOCKET) {
		shutdown(client->socket, SD_BOTH);
		closesocket(client->socket);
	}
}
