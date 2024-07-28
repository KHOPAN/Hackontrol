#include <WS2tcpip.h>
#include <hackontrolpacket.h>
#include <hackontroljava.h>
#include <khopanjava.h>
#include <lmcons.h>
#include "stream.h"

#pragma warning(disable: 6258)

#define WAIT_MAXIMUM 5000

static BOOL sendInformationPacket(JNIEnv* const environment, const SOCKET clientSocket) {
	HDC context = GetDC(NULL);
	int width = GetDeviceCaps(context, HORZRES);
	int height = GetDeviceCaps(context, VERTRES);
	BYTE buffer[UNLEN + 9];
	DWORD pointer = 0;
	buffer[pointer++] = (width >> 24) & 0xFF;
	buffer[pointer++] = (width >> 16) & 0xFF;
	buffer[pointer++] = (width >> 8) & 0xFF;
	buffer[pointer++] = width & 0xFF;
	buffer[pointer++] = (height >> 24) & 0xFF;
	buffer[pointer++] = (height >> 16) & 0xFF;
	buffer[pointer++] = (height >> 8) & 0xFF;
	buffer[pointer++] = height & 0xFF;
	DWORD usernameSize = UNLEN + 1;

	if(!GetUserNameA(buffer + pointer, &usernameSize)) {
		HackontrolThrowWin32Error(environment, L"GetUserNameA");
		return FALSE;
	}

	pointer += usernameSize - 1;
	PACKET packet;
	packet.size = (long) pointer;
	packet.packetType = PACKET_TYPE_INFORMATION;
	packet.data = buffer;

	if(!SendPacket(clientSocket, &packet)) {
		HackontrolThrowWin32Error(environment, L"SendPacket");
		return FALSE;
	}

	return TRUE;
}

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
	BOOL disconnect = FALSE;

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

	if(!sendInformationPacket(environment, clientSocket)) {
		goto closeSocket;
	}

	JavaVM* virtualMachine;

	if((*environment)->GetJavaVM(environment, &virtualMachine) != JNI_OK) {
		SetLastError(ERROR_FUNCTION_FAILED);
		HackontrolThrowWin32Error(environment, L"JNIEnv::GetJavaVM");
		goto closeSocket;
	}

	STREAMPARAMETER* streamParameter = LocalAlloc(LMEM_FIXED, sizeof(STREAMPARAMETER));

	if(!streamParameter) {
		HackontrolThrowWin32Error(environment, L"LocalAlloc");
		goto closeSocket;
	}

	streamParameter->virtualMachine = virtualMachine;
	streamParameter->clientSocket = clientSocket;
	HANDLE screenStreamThread = CreateThread(NULL, 0, ScreenStreamThread, streamParameter, 0, NULL);

	if(!screenStreamThread) {
		HackontrolThrowWin32Error(environment, L"CreateThread");
		LocalFree(streamParameter);
		goto closeSocket;
	}

	if(!SetThreadPriority(screenStreamThread, THREAD_PRIORITY_HIGHEST)) {
		HackontrolThrowWin32Error(environment, L"SetThreadPriority");
		goto closeScreenStreamThread;
	}

	PACKET packet;

	while(TRUE) {
		if(!ReceivePacket(clientSocket, &packet)) {
			HackontrolThrowWin32Error(environment, L"ReceivePacket");
			goto closeScreenStreamThread;
		}

		switch(packet.packetType) {
		case PACKET_TYPE_INFORMATION:
			SetStreamParameter(1, 0);
			disconnect = TRUE;
			goto closeScreenStreamThread;
		case PACKET_TYPE_STREAM_FRAME:
			SetStreamParameter(0, packet.data ? ((unsigned char*) packet.data)[0] : 0);
			break;
		}

		if(packet.data) {
			LocalFree(packet.data);
		}
	}
closeScreenStreamThread:
	WaitForSingleObject(screenStreamThread, WAIT_MAXIMUM);
	TerminateThread(screenStreamThread, 0);
	CloseHandle(screenStreamThread);
closeSocket:
	closesocket(clientSocket);
wsaCleanup:
	WSACleanup();

	if(disconnect) {
		(*environment)->CallObjectMethod(environment, callback, acceptMethod, (*environment)->NewStringUTF(environment, "**Disconnected**"));
	}
}
