#include "thread_client.h"
#include <khopanwin32.h>
#include <khopanarray.h>
#include "window_main.h"
#include "logger.h"

extern ArrayList clientList;

DWORD WINAPI ClientThread(_In_ PCLIENT client) {
	if(!client) {
		LOG("[Client Thread]: Exiting with an error: No client structure provided\n");
		return 1;
	}

	LOG("[Client Thread %ws]: Hello from client thread\n" COMMA client->address);
	char buffer[17];

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
	int returnValue = 1;

	if(!KHArrayAdd(&clientList, client)) {
		KHWin32DialogErrorW(GetLastError(), L"KHArrayAdd");
		goto exit;
	}

	RefreshMainWindowListView();

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
	CloseHandle(client->thread);
	LOG("[Client Thread %ws]: Exiting the client thread (Exit code: %d)\n" COMMA client->address COMMA returnValue);
	LocalFree(client);
	return returnValue;
}
