#include "thread_client.h"
#include <khopanwin32.h>
#include <khopanstring.h>
#include <khopanarray.h>
#include <hackontrolpacket.h>
#include "thread_window.h"
#include "window_main.h"
#include "frame_decoder.h"
#include "logger.h"

extern ArrayList clientList;
extern HANDLE listMutex;

static LPWSTR decodeName(const BYTE* data, long size) {
	if(size < 1) {
		return NULL;
	}

	LPSTR buffer = LocalAlloc(LMEM_FIXED, size + 1);

	if(!buffer) {
		return NULL;
	}

	for(int i = 0; i < size; i++) {
		buffer[i] = data[i];
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

	client->name = decodeName(packet.data, packet.size);
	LocalFree(packet.data);
	LOG("[Client Thread %ws]: Username: '%ws'\n" COMMA client->address COMMA client->name);
	WaitForSingleObject(listMutex, INFINITE);
	BOOL result = KHArrayAdd(&clientList, client);

	if(!ReleaseMutex(listMutex)) {
		KHWin32DialogErrorW(GetLastError(), L"ReleaseMutex");
		goto exit;
	}

	if(!result) {
		KHWin32DialogErrorW(GetLastError(), L"KHArrayAdd");
		goto exit;
	}

	LocalFree(client);
	WaitForSingleObject(listMutex, INFINITE);
	result = KHArrayGet(&clientList, clientList.elementCount - 1, &client);
	RefreshMainWindowListView();

	if(!ReleaseMutex(listMutex)) {
		KHWin32DialogErrorW(GetLastError(), L"ReleaseMutex");
		goto exit;
	}

	if(!result) {
		KHWin32DialogErrorW(GetLastError(), L"KHArrayGet");
		goto exit;
	}

	while(ReceivePacket(client->socket, &packet)) {
		if(packet.size < 1) {
			continue;
		}

		switch(packet.packetType) {
		case PACKET_TYPE_STREAM_FRAME: {
			DecodeHRSPFrame(packet.data, packet.size, client->stream, client->clientWindow);
			break;
		}
		default:
			LOG("[Client Thread %ws]: Unknown packet type: %d\n" COMMA client->address COMMA packet.packetType);
			break;
		}

		LocalFree(packet.data);
	}

	if(client->windowThread) {
		ExitClientWindow(client);
		LOG("[Client Thread %ws]: Waiting for window thread to exit\n" COMMA client->address);
		WaitForSingleObject(client->windowThread, INFINITE);
	}

	closesocket(client->socket);

	if(client->name) {
		LocalFree(client->name);
	}

	returnValue = 0;
exit:
	LOG("[Client Thread %ws]: Exiting the client thread (Exit code: %d)\n" COMMA client->address COMMA returnValue);
	CloseHandle(client->thread);
	client->active = FALSE;
	WaitForSingleObject(listMutex, INFINITE);
	RefreshMainWindowListView();

	if(!ReleaseMutex(listMutex)) {
		KHWin32DialogErrorW(GetLastError(), L"ReleaseMutex");
		return 1;
	}

	return returnValue;
}

void ClientOpen(PCLIENT client) {
	LOG("[Hackontrol Remote]: Opening %ws\n" COMMA client->address);

	if(client->windowThread) {
		ExitClientWindow(client);
		WaitForSingleObject(client->windowThread, INFINITE);
		client->windowThread = NULL;
	}

	client->windowThread = CreateThread(NULL, 0, WindowThread, client, 0, NULL);

	if(!client->windowThread) {
		KHWin32DialogErrorW(GetLastError(), L"CreateThread");
	}
}

void ClientDisconnect(PCLIENT client) {
	PACKET packet = {0};
	packet.packetType = PACKET_TYPE_INFORMATION;

	if(!SendPacket(client->socket, &packet)) {
		KHWin32DialogErrorW(GetLastError(), L"SendPacket");
		return;
	}

	if(shutdown(client->socket, SD_BOTH) == SOCKET_ERROR) {
		KHWin32DialogErrorW(WSAGetLastError(), L"shutdown");
		return;
	}

	if(closesocket(client->socket) == SOCKET_ERROR) {
		KHWin32DialogErrorW(WSAGetLastError(), L"closesocket");
	}
}
