#include "connection.h"
#include <khopanstring.h>
#include <khopanwin32.h>
#include <hackontrolpacket.h>

static LPWSTR getUsername(const PACKET* packet) {
	int size = packet->size - 8;

	if(size < 1) {
		return NULL;
	}

	LPSTR buffer = LocalAlloc(LMEM_FIXED, size + 1);

	if(!buffer) {
		return NULL;
	}

	BYTE* data = packet->data;

	for(int i = 0; i < size; i++) {
		buffer[i] = data[i + 8];
	}

	buffer[size] = 0;
	return KHFormatMessageW(L"%S", buffer);
}

DWORD WINAPI ClientThread(_In_ CLIENTENTRY* parameter) {
	if(!parameter) {
		return 1;
	}

	char buffer[17];

	if(recv(parameter->clientSocket, buffer, 16, 0) == SOCKET_ERROR) {
		KHWin32DialogErrorW(WSAGetLastError(), L"recv");
		goto closeSocket;
	}

	buffer[16] = 0;

	if(strcmp(buffer, "HRSP 1.0 CONNECT")) {
		MessageBoxW(NULL, L"The client has requested an invalid request", L"Hackontrol Remote", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
		goto closeSocket;
	}

	const char* header = "HRSP 1.0 OK";

	if(send(parameter->clientSocket, header, (int) strlen(header), 0) == SOCKET_ERROR) {
		KHWin32DialogErrorW(WSAGetLastError(), L"send");
		goto closeSocket;
	}

	PACKET packet;

	if(!ReceivePacket(parameter->clientSocket, &packet)) {
		KHWin32DialogErrorW(GetLastError(), L"ReceivePacket");
		goto closeSocket;
	}

	if(packet.packetType != PACKET_TYPE_INFORMATION) {
		goto closeSocket;
	}

	BYTE* data = packet.data;
	int width = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
	int height = (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7];
	parameter->username = getUsername(&packet);
	RemoteRefreshClientList();

	while(ReceivePacket(parameter->clientSocket, &packet)) {
		switch(packet.packetType) {
		case PACKET_TYPE_STREAM_FRAME:
			break;
		}
	}
closeSocket:
	RemoteRemoveEntry(parameter->clientSocket);
	closesocket(parameter->clientSocket);
	RemoteRefreshClientList();
	return 0;
}
