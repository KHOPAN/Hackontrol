#include "connection.h"

DWORD WINAPI ClientThread(_In_ SOCKET* parameter) {
	if(!parameter) {
		return 1;
	}

	SOCKET clientSocket = *((SOCKET*) parameter);
	closesocket(clientSocket);
	return 0;
}
