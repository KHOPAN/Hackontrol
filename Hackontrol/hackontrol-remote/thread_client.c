#include "thread_window.h"
#include <khopanwin32.h>
#include <khopanstring.h>
#include <khopanarray.h>
#include <hackontrolpacket.h>
#include "frame_decoder.h"
#include "logger.h"
#include "window_main.h"
#include <hrsp_handshake.h>
#include <hrsp_packet.h>
#include <hrsp_remote.h>

#define HRSPERROR(function) message=HRSPGetErrorMessage(function,&protocolError);if(message){LOG("[Client %ws]: %ws" COMMA client->address COMMA message);}

#pragma warning(disable: 6001)

extern ArrayList clients;
extern HANDLE clientsLock;

DWORD WINAPI ClientThread(_In_ PCLIENT client) {
	if(!client) {
		LOG("[Client]: No client structure provided\n");
		return 1;
	}

	LOG("[Client %ws]: Starting\n" COMMA client->address);
	int returnValue = 1;
	HRSPDATA protocolData;
	HRSPERROR protocolError;
	LPWSTR message;

	if(!HRSPServerHandshake(client->socket, &protocolData, &protocolError)) {
		HRSPERROR(L"HRSPServerHandshake");
		goto cleanupResource;
	}

	HRSPPACKET packet;

	if(!HRSPReceivePacket(client->socket, &protocolData, &packet, &protocolError)) {
		HRSPERROR(L"HRSPReceivePacket");
		goto cleanupResource;
	}

	if(packet.type != HRSP_REMOTE_CLIENT_INFORMATION_PACKET) {
		LOG("[Client %ws]: Invalid first packet type: %u\n" COMMA client->address COMMA packet.type);
		HRSPFreePacket(&packet, NULL);
		goto cleanupResource;
	}

	client->name = KHFormatMessageW(L"%S", packet.data);
	HRSPFreePacket(&packet, NULL);
	returnValue = 0;
	/*char buffer[17];
	int returnValue = 1;

	if(recv(client->socket, buffer, sizeof(buffer) - 1, 0) == SOCKET_ERROR) {
		KHWIN32_LAST_WSA_ERROR(L"recv");
		goto cleanupResource;
	}

	buffer[16] = 0;

	if(strcmp(buffer, "HRSP 1.0 CONNECT")) {
		MessageBoxW(NULL, L"The client has requested an invalid request", L"Remote", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
		LOG("[Client %ws]: Invalid request: %s\n" COMMA client->address COMMA buffer);
		goto cleanupResource;
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
		KHWIN32_LAST_WSA_ERROR(L"send");
		goto cleanupResource;
	}

	LOG("[Client %ws]: Completed HRSP Handshake\n" COMMA client->address);
	PACKET packet;

	if(!ReceivePacket(client->socket, &packet)) {
		KHWIN32_LAST_ERROR(L"ReceivePacket");
		goto cleanupResource;
	}

	if(packet.packetType != PACKET_TYPE_INFORMATION) {
		LOG("[Client %ws]: Invalid first packet type: %d\n" COMMA client->address COMMA packet.packetType);

		if(packet.size > 0) {
			LocalFree(packet.data);
		}

		goto cleanupResource;
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
		KHWIN32_LAST_ERROR(L"WaitForSingleObject");
		goto freeName;
	}

	if(!KHArrayAdd(&clients, client)) {
		KHWIN32_LAST_ERROR(L"KHArrayAdd");
		ReleaseMutex(clientsLock);
		goto freeName;
	}

	LocalFree(client);

	if(!KHArrayGet(&clients, clients.elementCount - 1, &client)) {
		KHWIN32_LAST_ERROR(L"KHArrayGet");
		ReleaseMutex(clientsLock);
		goto freeName;
	}

	if(!ReleaseMutex(clientsLock)) {
		KHWIN32_LAST_ERROR(L"ReleaseMutex");
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
			KHWIN32_LAST_ERROR(L"WaitForSingleObject");
			LocalFree(client->window);
			goto freeName;
		}

		if(!ReleaseMutex(client->window->lock)) {
			KHWIN32_LAST_ERROR(L"ReleaseMutex");
			LocalFree(client->window);
			goto freeName;
		}

		ClientWindowExit(client);

		if(WaitForSingleObject(client->window->thread, INFINITE) == WAIT_FAILED) {
			KHWIN32_LAST_ERROR(L"WaitForSingleObject");
			goto freeName;
		}
	}

	returnValue = 0;
freeName:
	if(client->name) {
		LocalFree(client->name);
	}*/
cleanupResource:
	if(client->thread) {
		CloseHandle(client->thread);
		client->thread = NULL;
	}

	if(client->socket) {
		closesocket(client->socket);
		client->socket = 0;
	}

	LOG("[Client %ws]: Exit client with code: %d\n" COMMA client->address COMMA returnValue);
	client->active = FALSE;
	MainWindowRefreshListView();
	return returnValue;
}

void ClientOpen(const PCLIENT client) {
	LOG("[Remote]: Opening %ws\n" COMMA client->address);

	if(client->window) {
		if(WaitForSingleObject(client->window->lock, INFINITE) == WAIT_FAILED) {
			LocalFree(client->window);
			client->window = NULL;
			return;
		}

		if(!ReleaseMutex(client->window->lock)) {
			LocalFree(client->window);
			client->window = NULL;
			return;
		}

		ClientWindowExit(client);

		if(WaitForSingleObject(client->window->thread, INFINITE) == WAIT_FAILED) {
			return;
		}
	}

	client->window = LocalAlloc(LMEM_FIXED, sizeof(WINDOWDATA));

	if(!client->window) {
		KHWIN32_LAST_ERROR(L"CreateThread");
		return;
	}

	client->window->lock = CreateMutexExW(NULL, NULL, 0, SYNCHRONIZE | DELETE);

	if(!client->window->lock) {
		KHWIN32_LAST_ERROR(L"CreateMutexExW");
		LocalFree(client->window);
		return;
	}

	client->window->thread = CreateThread(NULL, 0, ClientWindowThread, client, 0, NULL);

	if(!client->window->thread) {
		KHWIN32_LAST_ERROR(L"CreateThread");
		CloseHandle(client->window->lock);
		LocalFree(client->window);
	}
}

void ClientDisconnect(const PCLIENT client) {
	PACKET packet = {0};
	packet.packetType = PACKET_TYPE_INFORMATION;

	if(!SendPacket(client->socket, &packet)) {
		KHWIN32_LAST_ERROR(L"SendPacket");
		return;
	}

	if(shutdown(client->socket, SD_BOTH) == SOCKET_ERROR) {
		KHWIN32_LAST_WSA_ERROR(L"shutdown");
		return;
	}

	if(closesocket(client->socket) == SOCKET_ERROR) {
		KHWIN32_LAST_WSA_ERROR(L"closesocket");
	}
}
