#include "thread_window.h"
#include <khopanwin32.h>
#include <khopanstring.h>
#include <khopanarray.h>
#include <hackontrolpacket.h>
#include "frame_decoder.h"
#include "logger.h"
#include "window_main.h"

#pragma warning(disable: 6001)

extern ArrayList clients;
extern HANDLE clientsLock;

DWORD WINAPI ClientThread(_In_ PCLIENT client) {
	if(!client) {
		LOG("[Client]: No client structure provided\n");
		return 1;
	}

	LOG("[Client %ws]: Starting\n" COMMA client->address);
	char buffer[17];
	int returnValue = 1;

	if(recv(client->socket, buffer, sizeof(buffer) - 1, 0) == SOCKET_ERROR) {
		KHWin32DialogErrorW(WSAGetLastError(), L"recv");
		goto closeSocket;
	}

	buffer[16] = 0;

	if(strcmp(buffer, "HRSP 1.0 CONNECT")) {
		MessageBoxW(NULL, L"The client has requested an invalid request", L"Remote", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
		LOG("[Client %ws]: Invalid request: %s\n" COMMA client->address COMMA buffer);
		goto closeSocket;
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
		goto closeSocket;
	}

	LOG("[Client %ws]: Completed HRSP Handshake\n" COMMA client->address);
	PACKET packet;

	if(!ReceivePacket(client->socket, &packet)) {
		KHWin32DialogErrorW(GetLastError(), L"ReceivePacket");
		goto closeSocket;
	}

	if(packet.packetType != PACKET_TYPE_INFORMATION) {
		LOG("[Client %ws]: Invalid first packet type: %d\n" COMMA client->address COMMA packet.packetType);

		if(packet.size > 0) {
			LocalFree(packet.data);
		}

		goto closeSocket;
	}

	if(packet.size > 0) {
		BYTE* nameBuffer = LocalAlloc(LMEM_FIXED, packet.size + 1);

		if(!nameBuffer) {
			goto exitName;
		}

		for(long i = 0; i < packet.size; i++) {
			nameBuffer[i] = ((BYTE*) packet.data)[i];
		}

		LocalFree(packet.data);
		nameBuffer[packet.size] = 0;
		client->name = KHFormatMessageW(L"%S", nameBuffer);
		LocalFree(nameBuffer);
	}
exitName:
	LOG("[Client %ws]: Username: '%ws'\n" COMMA client->address COMMA client->name);

	if(WaitForSingleObject(clientsLock, INFINITE) == WAIT_FAILED) {
		KHWin32DialogErrorW(GetLastError(), L"WaitForSingleObject");
		goto freeName;
	}

	if(!KHArrayAdd(&clients, client)) {
		KHWin32DialogErrorW(GetLastError(), L"KHArrayAdd");
		ReleaseMutex(clientsLock);
		goto freeName;
	}

	LocalFree(client);

	if(!KHArrayGet(&clients, clients.elementCount - 1, &client)) {
		KHWin32DialogErrorW(GetLastError(), L"KHArrayGet");
		ReleaseMutex(clientsLock);
		goto freeName;
	}

	if(!ReleaseMutex(clientsLock)) {
		KHWin32DialogErrorW(GetLastError(), L"ReleaseMutex");
		goto freeName;
	}

	MainWindowRefreshListView();

	while(ReceivePacket(client->socket, &packet)) {
		if(packet.size < 1) {
			continue;
		}

		switch(packet.packetType) {
		case PACKET_TYPE_STREAM_FRAME: {
			DecodeHRSPFrame(packet.data, packet.size, client->window);
			break;
		}
		default:
			LOG("[Client %ws]: Unknown packet type: %d\n" COMMA client->address COMMA packet.packetType);
			break;
		}

		LocalFree(packet.data);
	}

	if(client->window) {
		LOG("[Client %ws]: Wait for window thread to exit\n" COMMA client->address);

		if(WaitForSingleObject(client->window->lock, INFINITE) == WAIT_FAILED) {
			KHWin32DialogErrorW(GetLastError(), L"WaitForSingleObject");
			goto freeName;
		}

		ClientWindowExit(client);
		CloseHandle(client->window->lock);

		if(WaitForSingleObject(client->window->thread, INFINITE) == WAIT_FAILED) {
			KHWin32DialogErrorW(GetLastError(), L"WaitForSingleObject");
			goto freeName;
		}
	}

	returnValue = 0;
freeName:
	if(client->name) {
		LocalFree(client->name);
	}
closeSocket:
	CloseHandle(client->thread);
	client->thread = NULL;
	closesocket(client->socket);
	LOG("[Client %ws]: Exit client with code: %d\n" COMMA client->address COMMA returnValue);
	client->active = FALSE;
	MainWindowRefreshListView();
	return returnValue;
}

void ClientOpen(const PCLIENT client) {
	LOG("[Remote]: Opening %ws\n" COMMA client->address);

	if(client->window) {
		if(WaitForSingleObject(client->window->lock, INFINITE) == WAIT_FAILED) {
			client->window = NULL;
			return;
		}

		ClientWindowExit(client);
		CloseHandle(client->window->lock);

		if(WaitForSingleObject(client->window->thread, INFINITE) == WAIT_FAILED) {
			client->window = NULL;
			return;
		}
	}

	client->window = LocalAlloc(LMEM_FIXED, sizeof(WINDOWDATA));

	if(!client->window) {
		KHWin32DialogErrorW(GetLastError(), L"CreateThread");
		return;
	}

	client->window->lock = CreateMutexExW(NULL, NULL, 0, SYNCHRONIZE | DELETE);

	if(!client->window->lock) {
		KHWin32DialogErrorW(GetLastError(), L"CreateMutexExW");
		LocalFree(client->window);
		return;
	}

	client->window->thread = CreateThread(NULL, 0, ClientWindowThread, client, 0, NULL);

	if(!client->window->thread) {
		KHWin32DialogErrorW(GetLastError(), L"CreateThread");
		CloseHandle(client->window->lock);
		LocalFree(client->window);
	}
}

void ClientDisconnect(const PCLIENT client) {
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
