#include <WinSock2.h>
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

	LOG("[Client %ws]: Initializing\n", client->address);
	HRSPERROR protocolError;
	LPWSTR message;
	PLINKEDLISTITEM item = NULL;
	DWORD codeExit = 1;

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
		KHOPANLASTERRORCONSOLE_WIN32(L"WaitForSingleObject");
		goto freeName;
	}

	if(!KHOPANLinkedAdd(&clientList, (PBYTE) client, &item)) {
		KHOPANLASTERRORCONSOLE_WIN32(L"KHOPANLinkedAdd");
		ReleaseMutex(clientListMutex);
		goto freeName;
	}

	KHOPAN_DEALLOCATE(client);
	client = (PCLIENT) item->data;
	WindowMainRefresh();
	ReleaseMutex(clientListMutex);

	while(HRSPReceivePacket(client->socket, &client->hrsp, &packet, &protocolError)) {
		switch(packet.type) {
		default:
			LOG("[Client %ws]: Unknown packet type: %u\n", client->address, packet.type);
			break;
		}

		HRSPFreePacket(&packet, NULL);
	}

	if(!protocolError.code || (!protocolError.win32 && protocolError.code == HRSP_ERROR_CONNECTION_CLOSED) || (protocolError.win32 && (protocolError.code == WSAEINTR || protocolError.code == WSAECONNABORTED || protocolError.code == WSAECONNRESET))) {
		codeExit = 0;
		goto closeSession;
	}

	ERROR_HRSP(L"HRSPReceivePacket");
closeSession:
	if(client->session.thread) {
		WaitForSingleObject(client->session.mutex, INFINITE);
		WindowSessionClose(client);
		CloseHandle(client->session.mutex);
		WaitForSingleObject(client->session.thread, INFINITE);
	}
freeName:
	if(client->name) {
		LocalFree(client->name);
	}
functionExit:
	closesocket(client->socket);
	LOG("[Client %ws]: Exit with code: %d\n", client->address, codeExit);
	CloseHandle(client->thread);

	if(item && WaitForSingleObject(clientListMutex, INFINITE) != WAIT_FAILED) {
		if(KHOPANLinkedRemove(item)) {
			WindowMainRefresh();
		} else {
			KHOPANLASTERRORCONSOLE_WIN32(L"KHOPANLinkedRemove");
			codeExit = 1;
		}

		ReleaseMutex(clientListMutex);
	}

	return codeExit;
}

void ThreadClientOpen(const PCLIENT client) {
	if(!client) {
		return;
	}

	if(client->session.mutex) {
		WaitForSingleObject(client->session.mutex, INFINITE);
	} else {
		client->session.mutex = CreateMutexExW(NULL, NULL, 0, SYNCHRONIZE | DELETE);

		if(!client->session.mutex) {
			KHOPANLASTERRORCONSOLE_WIN32(L"CreateMutexExW");
			return;
		}

		WaitForSingleObject(client->session.mutex, INFINITE);
	}

	if(client->session.thread) {
		WindowSessionClose(client);
	}

	ReleaseMutex(client->session.mutex);

	if(client->session.thread) {
		WaitForSingleObject(client->session.thread, INFINITE);
	}

	client->session.thread = CreateThread(NULL, 0, WindowSession, client, 0, NULL);

	if(!client->session.thread) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateThread");
		CloseHandle(client->session.mutex);
	}
}

void ThreadClientDisconnect(const PCLIENT client) {
	if(client && client->socket) {
		shutdown(client->socket, SD_BOTH);
		closesocket(client->socket);
	}
}
