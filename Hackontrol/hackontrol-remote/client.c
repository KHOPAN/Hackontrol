#include "connection.h"
#include <khopanstring.h>
#include <khopanwin32.h>
#include "packet.h"

static LPWSTR getUsername(const PACKET packet) {
	int size = packet.size - 8;

	if(size < 1) {
		return NULL;
	}

	LPSTR buffer = LocalAlloc(LMEM_FIXED, size + 1);

	if(!buffer) {
		return NULL;
	}

	BYTE* data = packet.data;

	for(int i = 0; i < size; i++) {
		buffer[i] = data[i + 8];
	}

	buffer[size] = 0;
	return KHFormatMessageW(L"%S", buffer);
}

DWORD WINAPI ClientThread(_In_ SOCKET* parameter) {
	if(!parameter) {
		return 1;
	}

	SOCKET clientSocket = *((SOCKET*) parameter);
	char buffer[17];

	if(recv(clientSocket, buffer, 16, 0) == SOCKET_ERROR) {
		KHWin32DialogErrorW(WSAGetLastError(), L"recv");
		goto closeSocket;
	}

	buffer[16] = 0;

	if(strcmp(buffer, "HRSP 1.0 CONNECT")) {
		MessageBoxW(NULL, L"The client has requested an invalid request", L"Hackontrol Remote", MB_OK | MB_DEFBUTTON1 | MB_ICONERROR | MB_SYSTEMMODAL);
		goto closeSocket;
	}

	const char* header = "HRSP 1.0 OK";

	if(send(clientSocket, header, (int) strlen(header), 0) == SOCKET_ERROR) {
		KHWin32DialogErrorW(WSAGetLastError(), L"send");
		goto closeSocket;
	}

	PACKET packet;

	if(!ReceivePacket(clientSocket, &packet)) {
		KHWin32DialogErrorW(GetLastError(), L"ReceivePacket");
		goto closeSocket;
	}

	BYTE* data = packet.data;
	LPWSTR username = getUsername(packet);
	int width = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
	int height = (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7];
	MessageBoxW(NULL, username, L"Remote", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	if(username) LocalFree(username);
closeSocket:
	closesocket(clientSocket);
	return 0;
}