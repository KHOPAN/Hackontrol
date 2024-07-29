#include "thread_client.h"
#include <khopanwin32.h>
#include <khopanstring.h>
#include <khopanarray.h>
#include <hackontrolpacket.h>
#include "window_main.h"
#include "logger.h"

extern ArrayList clientList;

static LPWSTR decodeName(const BYTE* data, long size) {
	size -= 8;

	if(size < 1) {
		return NULL;
	}

	LPSTR buffer = LocalAlloc(LMEM_FIXED, size + 1);

	if(!buffer) {
		return NULL;
	}

	for(int i = 0; i < size; i++) {
		buffer[i] = data[i + 8];
	}

	buffer[size] = 0;
	return KHFormatMessageW(L"%S", buffer);
}

DWORD WINAPI ClientThread(_In_ PCLIENT client) {
	if(!client) {
		LOG("[Client Thread]: Exiting with an error: No client structure provided\n");
		return 1;
	}

	LOG("[Client Thread %ws]: Hello from client thread\n" COMMA client->address);
	char buffer[17];
	int returnValue = 1;

	if(recv(client->socket, buffer, sizeof(buffer) - 1, 0) == SOCKET_ERROR) {
		KHWin32DialogErrorW(WSAGetLastError(), L"recv");
		goto exit;
	}

	buffer[16] = 0;

	if(strcmp(buffer, "HRSP 1.0 CONNECT")) {
		MessageBoxW(NULL, L"The client has requested an invalid request", L"Hackontrol Remote", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
		LOG("[Client Thread %ws]: Client has requested an invalid request: %s\n" COMMA client->address COMMA buffer);
		goto exit;
	}

	buffer[0] = 'H';
	buffer[1] = 'R';
	buffer[2] = 'S';
	buffer[3] = 'P';
	buffer[4] = ' ';
	buffer[5] = '1';
	buffer[6] = '.';
	buffer[7] = '0';
	buffer[8] = ' ';
	buffer[9] = 'O';
	buffer[10] = 'K';

	if(send(client->socket, buffer, 11, 0) == SOCKET_ERROR) {
		KHWin32DialogErrorW(WSAGetLastError(), L"send");
		goto exit;
	}

	LOG("[Client Thread %ws]: HRSP handshake completed! Receiving the first packet...\n" COMMA client->address);
	PACKET packet;

	if(!ReceivePacket(client->socket, &packet)) {
		KHWin32DialogErrorW(GetLastError(), L"ReceivePacket");
		goto exit;
	}

	if(packet.packetType != PACKET_TYPE_INFORMATION) {
		LOG("[Client Thread %ws]: Closing the connection, invalid first packet type: %d\n" COMMA client->address COMMA packet.packetType);
		goto exit;
	}

	BYTE* data = packet.data;
	int width = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
	int height = (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7];
	client->name = decodeName(data, packet.size);
	LOG("[Client Thread %ws]: Username: '%ws' Screen: %dx%d\n" COMMA client->address COMMA client->name COMMA width COMMA height);

	if(!KHArrayAdd(&clientList, client)) {
		KHWin32DialogErrorW(GetLastError(), L"KHArrayAdd");
		goto exit;
	}

	RefreshMainWindowListView();

	while(ReceivePacket(client->socket, &packet)) {
		switch(packet.packetType) {
		case PACKET_TYPE_STREAM_FRAME:
			continue;
		}

		LOG("[Client Thread %ws]: Unknown packet type: %d\n" COMMA client->address COMMA packet.packetType);
	}

	for(size_t i = 0; i < clientList.elementCount; i++) {
		PCLIENT instance;

		if(!KHArrayGet(&clientList, i, &instance)) {
			KHWin32DialogErrorW(GetLastError(), L"KHArrayGet");
			break;
		}

		if(instance->socket != client->socket) {
			continue;
		}

		if(!KHArrayRemove(&clientList, i)) {
			KHWin32DialogErrorW(GetLastError(), L"KHArrayRemove");
			break;
		}

		RefreshMainWindowListView();
		returnValue = 0;
		goto exit;
	}

	LOG("[Client Thread]: Error: Client not found in the client list\n");
exit:
	closesocket(client->socket);

	if(client->name) {
		LocalFree(client->name);
	}

	HANDLE thread = client->thread;
	LOG("[Client Thread %ws]: Exiting the client thread (Exit code: %d)\n" COMMA client->address COMMA returnValue);
	LocalFree(client);
	CloseHandle(thread);
	return returnValue;
}
