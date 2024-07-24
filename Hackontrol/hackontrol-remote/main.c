#include <WS2tcpip.h>
#include <khopanwin32.h>
#include "window.h"

#define REMOTE_PORT "42485"

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE previousInstance, _In_ LPSTR argument, _In_ int commandLineShow) {
	HANDLE mainWindowThread = CreateThread(NULL, 0, MainWindowThread, NULL, 0, NULL);

	if(!mainWindowThread) {
		KHWin32DialogErrorW(GetLastError(), L"CreateThread");
		return 1;
	}

	WSADATA windowsSocketData;
	int status = WSAStartup(MAKEWORD(2, 2), &windowsSocketData);
	int returnValue = 1;

	if(status) {
		KHWin32DialogErrorW(status, L"WSAStartup");
		goto closeMainWindowThread;
	}

	struct addrinfo hints = {0};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	struct addrinfo* result;
	status = getaddrinfo(NULL, REMOTE_PORT, &hints, &result);

	if(status) {
		KHWin32DialogErrorW(status, L"getaddrinfo");
		goto wsaCleanup;
	}

	SOCKET serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if(serverSocket == INVALID_SOCKET) {
		KHWin32DialogErrorW(WSAGetLastError(), L"socket");
		freeaddrinfo(result);
		goto wsaCleanup;
	}

	status = bind(serverSocket, result->ai_addr, (int) result->ai_addrlen);
	freeaddrinfo(result);

	if(status == SOCKET_ERROR) {
		KHWin32DialogErrorW(WSAGetLastError(), L"bind");
		goto closeServerSocket;
	}

	if(listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
		KHWin32DialogErrorW(WSAGetLastError(), L"listen");
		goto closeServerSocket;
	}

	SOCKET clientSocket = accept(serverSocket, NULL, NULL);

	if(clientSocket == INVALID_SOCKET) {
		KHWin32DialogErrorW(WSAGetLastError(), L"accept");
		goto closeServerSocket;
	}

	MessageBoxW(NULL, L"Connected", L"Hackontrol Remote", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	returnValue = 0;
	closesocket(clientSocket);
closeServerSocket:
	closesocket(serverSocket);
wsaCleanup:
	if(WSACleanup() == SOCKET_ERROR) {
		KHWin32DialogErrorW(status, L"WSACleanup");
		returnValue = 1;
	}
closeMainWindowThread:
	CloseHandle(mainWindowThread);
	return returnValue;
}
