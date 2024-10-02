#include <libkhopanlist.h>
#include <hrsp_handshake.h>
#include <hrsp_packet.h>
#include <hrsp_remote.h>
#include "remote.h"

#define ERROR_HRSP(function) message=HRSPGetErrorMessage(function,&protocolError);if(message){LOG("[Client %ws]: %ws",client->address,message);LocalFree(message);}

extern LINKEDLIST clientList;
extern HANDLE clientListMutex;

DWORD WINAPI ThreadClient(_In_ PCLIENT client) {
	if(!client) {
		LOG("[Client]: Empty thread parameter\n");
		return 1;
	}

	DWORD codeExit = 1;
	LOG("[Client %ws]: Initializing\n", client->address);
	HRSPERROR protocolError;
	LPWSTR message;

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

	if(WaitForSingleObject(clientListMutex, INFINITE) == WAIT_FAILED) {
		KHOPANLASTERRORMESSAGE_WIN32(L"WaitForSingleObject");
		goto freeName;
	}

	PLINKEDLISTITEM item;

	if(!KHOPANLinkedAdd(&clientList, (PBYTE) client, &item)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANLinkedAdd");
		ReleaseMutex(clientListMutex);
		goto freeName;
	}

	KHOPAN_DEALLOCATE(client);
	client = (PCLIENT) item->data;
	ReleaseMutex(clientListMutex);
	WindowMainRefresh();
	codeExit = 0;
freeName:
	if(client->name) {
		LocalFree(client->name);
	}
functionExit:
	LOG("[Client %ws]: Exit with code: %d\n", client->address, codeExit);
	CloseHandle(client->thread);
	return codeExit;
}
