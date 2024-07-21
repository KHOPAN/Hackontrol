#include <WS2tcpip.h>
#include <khopanjava.h>
#include "exception.h"
#include "screenshot.h"
#include "packet.h"

_declspec(dllexport) void __stdcall ConnectHRSPServer(JNIEnv* const environment, LPCSTR hostName, LPCSTR port, const jobject callback) {
	jclass consumerClass = (*environment)->FindClass(environment, "java/util/function/Consumer");
	
	if(!consumerClass) {
		return;
	}

	jmethodID acceptMethod = (*environment)->GetMethodID(environment, consumerClass, "accept", "(Ljava/lang/Object;)V");

	if(!acceptMethod) {
		return;
	}

	WSADATA windowsSocketData;
	int status = WSAStartup(MAKEWORD(2, 2), &windowsSocketData);

	if(status) {
		SetLastError(status);
		HackontrolThrowWin32Error(environment, L"WSAStartup");
		return;
	}

	struct addrinfo hints = {0};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	struct addrinfo* result;
	status = getaddrinfo(hostName, port, &hints, &result);

	if(status) {
		SetLastError(status);
		HackontrolThrowWin32Error(environment, L"getaddrinfo");
		goto wsaCleanup;
	}

	SOCKET clientSocket = INVALID_SOCKET;

	for(struct addrinfo* pointer = result; pointer != NULL; pointer = pointer->ai_next) {
		clientSocket = socket(pointer->ai_family, pointer->ai_socktype, pointer->ai_protocol);

		if(clientSocket == INVALID_SOCKET) {
			SetLastError(WSAGetLastError());
			HackontrolThrowWin32Error(environment, L"socket");
			freeaddrinfo(result);
			goto wsaCleanup;
		}

		status = connect(clientSocket, pointer->ai_addr, (int) pointer->ai_addrlen);

		if(status != SOCKET_ERROR) {
			break;
		}

		closesocket(clientSocket);
		clientSocket = INVALID_SOCKET;
	}

	freeaddrinfo(result);

	if(clientSocket == INVALID_SOCKET) {
		KHJavaThrowInternalErrorW(environment, L"Unable to connect to the server");
		goto wsaCleanup;
	}

	const char* header = "HRSP 1.0 CONNECT";
	status = send(clientSocket, header, (int) strlen(header), 0);

	if(status == SOCKET_ERROR) {
		SetLastError(WSAGetLastError());
		HackontrolThrowWin32Error(environment, L"send");
		goto closeSocket;
	}

	char buffer[12];
	status = recv(clientSocket, buffer, 11, 0);

	if(status == SOCKET_ERROR) {
		SetLastError(WSAGetLastError());
		HackontrolThrowWin32Error(environment, L"recv");
		goto closeSocket;
	}

	buffer[11] = 0;

	if(strcmp(buffer, "HRSP 1.0 OK")) {
		KHJavaThrowInternalErrorW(environment, L"Server responded with an invalid response");
		goto closeSocket;
	}

	(*environment)->CallObjectMethod(environment, callback, acceptMethod, (*environment)->NewStringUTF(environment, "**Connected**"));
	/*HDC context = GetDC(NULL);
	int width = GetDeviceCaps(context, HORZRES);
	int height = GetDeviceCaps(context, VERTRES);
	BYTE screenInfoBuffer[8];
	screenInfoBuffer[0] = (width >> 24) & 0xFF;
	screenInfoBuffer[1] = (width >> 16) & 0xFF;
	screenInfoBuffer[2] = (width >> 8) & 0xFF;
	screenInfoBuffer[3] = width & 0xFF;
	screenInfoBuffer[4] = (height >> 24) & 0xFF;
	screenInfoBuffer[5] = (height >> 16) & 0xFF;
	screenInfoBuffer[6] = (height >> 8) & 0xFF;
	screenInfoBuffer[7] = height & 0xFF;*/
	/*DWORD usernameSize = 0;

	if(!GetUserNameA(NULL, &usernameSize) && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
		HackontrolThrowWin32Error(environment, L"GetUserNameA");
		goto closeSocket;
	}

	LPSTR username = LocalAlloc(LMEM_FIXED, usernameSize);

	if(!username) {
		HackontrolThrowWin32Error(environment, L"LocalAlloc");
		goto closeSocket;
	}

	if(!GetUserNameA(username, &usernameSize)) {
		HackontrolThrowWin32Error(environment, L"GetUserNameA");
		LocalFree(username);
		goto closeSocket;
	}

	PACKET packet;
	packet.size = (long) (usernameSize - 1);
	packet.packetType = PACKET_TYPE_INFORMATION;
	packet.data = username;
	BOOL errorCode = SendPacket(clientSocket, &packet);
	LocalFree(username);

	if(!errorCode) {
		HackontrolThrowWin32Error(environment, L"SendPacket");
		goto closeSocket;
	}

	int width = 1366;
	int height = 768;
	size_t baseSize = width* height;
	size_t bufferSize = baseSize * 4;
	BYTE* screenshotBuffer = LocalAlloc(LMEM_FIXED, bufferSize);

	if(!screenshotBuffer) {
		HackontrolThrowWin32Error(environment, L"LocalAlloc");
		goto closeSocket;
	}

	BYTE* qoiBuffer = LocalAlloc(LMEM_FIXED, bufferSize);

	if(!qoiBuffer) {
		HackontrolThrowWin32Error(environment, L"LocalAlloc");
		goto freeScreenshotBuffer;
	}

	BYTE* previousBuffer = LocalAlloc(LMEM_FIXED, baseSize * 3);

	if(!previousBuffer) {
		HackontrolThrowWin32Error(environment, L"LocalAlloc");
		goto freeQOIBuffer;
	}

	while(TRUE) {
		if(!TakeScreenshot(environment, clientSocket, width, height, screenshotBuffer, qoiBuffer, previousBuffer)) {
			goto freePreviousBuffer;
		}
	}
freePreviousBuffer:
	LocalFree(previousBuffer);
freeQOIBuffer:
	LocalFree(qoiBuffer);
freeScreenshotBuffer:
	LocalFree(screenshotBuffer);*/
	PACKET packet;

	while(TRUE) {
		if(!ReceivePacket(clientSocket, &packet)) {
			HackontrolThrowWin32Error(environment, L"ReceivePacket");
			goto closeSocket;
		}

		printf("Data: %d\n", packet.size);
		_flushall();
		LocalFree(packet.data);
	}
closeSocket:
	closesocket(clientSocket);
wsaCleanup:
	WSACleanup();
}
