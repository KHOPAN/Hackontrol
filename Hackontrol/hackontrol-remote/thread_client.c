#include <WinSock2.h>
#include <hrsp_remote.h>
#include "remote.h"

extern LINKEDLIST clientList;
extern HANDLE clientListMutex;
extern HRSPSERVERDATA serverData;

DWORD WINAPI ThreadClient(_In_ PCLIENT client) {
	if(!client) {
		LOG("[Client]: Empty thread parameter\n");
		return 1;
	}

	LOG("[Client %ws]: Initializing\n", client->address);
	KHOPANERROR error;
	DWORD codeExit = 1;
	PLINKEDLISTITEM item = NULL;

	if(!HRSPServerSessionInitialize(client->socket, &client->hrsp, &serverData, &error)) {
		KHOPANERRORCONSOLE_KHOPAN(error);
		goto functionExit;
	}

	HRSPPACKET packet;

	if(!HRSPPacketReceive(&client->hrsp, &packet, &error)) {
		KHOPANERRORCONSOLE_KHOPAN(error);
		goto cleanupProtocol;
	}

	if(packet.type != HRSP_REMOTE_CLIENT_USERNAME) {
		LOG("[Client %ws]: Invalid first packet type: %u\n", client->address, packet.type);
		if(packet.size) KHOPAN_DEALLOCATE(packet.data);
		goto cleanupProtocol;
	}

	client->name = packet.data;
	LOG("[Client %ws]: Username: '%ws'\n", client->address, client->name);

	if(!WindowMainAdd(&client, &item)) {
		goto freeName;
	}

	while(HRSPPacketReceive(&client->hrsp, &packet, &error)) {
		if(!WindowSessionHandlePacket(client, &packet)) {
			LOG("[Client %ws]: Unknown packet type: %u\nData:", client->address, packet.type);
			for(size_t i = 0; i < packet.size; i++) LOG(" 0x%02X", ((PBYTE) packet.data)[i]);
			LOG("\n");
		}

		if(packet.size) {
			KHOPAN_DEALLOCATE(packet.data);
		}
	}

	if((error.facility == ERROR_FACILITY_HRSP && error.code == ERROR_HRSP_CONNECTION_CLOSED) || (error.facility == ERROR_FACILITY_WIN32 && (error.code == WSAECONNABORTED || error.code == WSAECONNRESET || error.code == WSAEINTR || error.code == WSAENOTSOCK))) {
		codeExit = 0;
		goto closeSession;
	}

	KHOPANERRORCONSOLE_KHOPAN(error);
closeSession:
	if(client->session.thread) {
		WindowSessionClose(client);
		WaitForSingleObject(client->session.thread, INFINITE);
	}
freeName:
	if(client->name) {
		KHOPAN_DEALLOCATE(client->name);
	}
cleanupProtocol:
	HRSPServerSessionCleanup(&client->hrsp);
functionExit:
	LOG("[Client %ws]: Exit with code: %d\n", client->address, codeExit);
	closesocket(client->socket);
	CloseHandle(client->thread);

	if(item) {
		if(!WindowMainRemove(item)) {
			codeExit = 1;
		}
	} else {
		KHOPAN_DEALLOCATE(client);
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
