#include <WS2tcpip.h>
#include <lmcons.h>
#include <hrsp_remote.h>
#include "hrsp_client_internal.h"

#define ERROR_WIN32(codeError, sourceName, functionName)  if(error){error->facility=ERROR_FACILITY_WIN32;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_WSA(sourceName, functionName)               if(error){error->facility=ERROR_FACILITY_WIN32;error->code=WSAGetLastError();error->source=sourceName;error->function=functionName;}
#define ERROR_COMMON(codeError, sourceName, functionName) if(error){error->facility=ERROR_FACILITY_COMMON;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_CLEAR                                       ERROR_COMMON(ERROR_COMMON_SUCCESS,NULL,NULL)
#define ERROR_SOURCE(sourceName)                          if(error){error->function=error->source;error->source=sourceName;}

#define BACKGROUND_WINDOW_CLASS L"HRSPClientBackground"

typedef struct {
	HWND window;
} BACKGROUNDTHREAD, *PBACKGROUNDTHREAD;

static LRESULT CALLBACK backgroundProcedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	printf("Message: %u\n", message);
	return DefWindowProcW(window, message, wparam, lparam);
}

static DWORD WINAPI backgroundThread(_In_ PBACKGROUNDTHREAD background) {
	if(!background) {
		return 1;
	}

	HINSTANCE instance = GetModuleHandleW(NULL);
	WNDCLASSW windowClass = {0};
	windowClass.lpfnWndProc = backgroundProcedure;
	windowClass.hInstance = instance;
	windowClass.lpszClassName = BACKGROUND_WINDOW_CLASS;

	if(!RegisterClassW(&windowClass)) {
		return 1;
	}

	background->window = CreateWindowExW(0, BACKGROUND_WINDOW_CLASS, NULL, 0, 0, 0, 0, 0, NULL, NULL, instance, NULL);

	if(!background->window) {
		UnregisterClassW(BACKGROUND_WINDOW_CLASS, instance);
		return 1;
	}

	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	UnregisterClassW(BACKGROUND_WINDOW_CLASS, instance);
	return 0;
}

BOOL HRSPClientConnectToServer(const LPCWSTR address, const LPCWSTR port, const PHRSPCLIENTINPUT input, const PKHOPANERROR error) {
	WSADATA data;
	int status = WSAStartup(MAKEWORD(2, 2), &data);

	if(status) {
		ERROR_WIN32(status, L"HRSPClientConnectToServer", L"WSAStartup");
		return 1;
	}

	ADDRINFOW hints = {0};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	BOOL codeExit = FALSE;

	if(GetAddrInfoW(address ? address : L"localhost", port ? port : HRSP_PROTOCOL_PORT_STRING, &hints, &hints.ai_next)) {
		ERROR_WSA(L"HRSPClientConnectToServer", L"GetAddrInfoW");
		goto cleanupSocket;
	}

	SOCKET clientSocket = INVALID_SOCKET;

	for(PADDRINFOW pointer = hints.ai_next; pointer != NULL; pointer = pointer->ai_next) {
		clientSocket = socket(pointer->ai_family, pointer->ai_socktype, pointer->ai_protocol);

		if(clientSocket == INVALID_SOCKET) {
			ERROR_WSA(L"HRSPClientConnectToServer", L"socket");
			FreeAddrInfoW(hints.ai_next);
			goto cleanupSocket;
		}

		if(connect(clientSocket, pointer->ai_addr, (int) pointer->ai_addrlen) != SOCKET_ERROR) {
			break;
		}

		closesocket(clientSocket);
		clientSocket = INVALID_SOCKET;
	}

	FreeAddrInfoW(hints.ai_next);

	if(clientSocket == INVALID_SOCKET) {
		ERROR_COMMON(ERROR_COMMON_FUNCTION_FAILED, L"HRSPClientConnectToServer", NULL);
		goto cleanupSocket;
	}

	HRSPDATA protocolData;

	if(!HRSPClientInitialize(clientSocket, &protocolData, error)) {
		ERROR_SOURCE(L"HRSPClientConnectToServer");
		goto closeSocket;
	}

	DWORD size = UNLEN + 1;
	PBYTE buffer = KHOPAN_ALLOCATE(size * sizeof(WCHAR));

	if(!buffer) {
		ERROR_COMMON(ERROR_COMMON_FUNCTION_FAILED, L"HRSPClientConnectToServer", L"KHOPAN_ALLOCATE");
		goto cleanupProtocol;
	}

	if(!GetUserNameW((LPWSTR) buffer, &size)) {
		ERROR_WIN32(GetLastError(), L"HRSPClientConnectToServer", L"GetUserNameW");
		KHOPAN_DEALLOCATE(buffer);
		goto cleanupProtocol;
	}

	HRSPPACKET packet;
	packet.type = HRSP_REMOTE_CLIENT_USERNAME;
	packet.size = size * sizeof(WCHAR);
	packet.data = buffer;
	size = HRSPPacketSend(&protocolData, &packet, error);
	KHOPAN_DEALLOCATE(buffer);

	if(!size) {
		ERROR_SOURCE(L"HRSPClientConnectToServer");
		goto cleanupProtocol;
	}

	buffer = KHOPAN_ALLOCATE(sizeof(BACKGROUNDTHREAD));

	if(!buffer) {
		ERROR_COMMON(ERROR_COMMON_FUNCTION_FAILED, L"HRSPClientConnectToServer", L"KHOPAN_ALLOCATE");
		goto cleanupProtocol;
	}

	HANDLE thread = CreateThread(NULL, 0, backgroundThread, buffer, 0, NULL);

	if(!thread) {
		ERROR_WIN32(GetLastError(), L"HRSPClientConnectToServer", L"CreateThread");
		KHOPAN_DEALLOCATE(buffer);
		goto cleanupProtocol;
	}

	if(input && input->callbackConnected) {
		input->callbackConnected(input->parameter);
	}

	while(HRSPPacketReceive(&protocolData, &packet, error)) {
		switch(packet.type) {
		case HRSP_REMOTE_SERVER_REQUEST_STREAM_DEVICE:
			StreamRequestDevice(clientSocket, &protocolData);
			break;
		}

		if(packet.data) {
			KHOPAN_DEALLOCATE(packet.data);
		}
	}

	PostMessageW(((PBACKGROUNDTHREAD) buffer)->window, WM_CLOSE, 0, 0);
	CloseHandle(thread);
	KHOPAN_DEALLOCATE(buffer);

	if(error->facility != ERROR_FACILITY_HRSP || error->code != ERROR_HRSP_CONNECTION_CLOSED) {
		ERROR_SOURCE(L"HRSPClientConnectToServer");
		goto cleanupProtocol;
	}

	codeExit = TRUE;
	ERROR_CLEAR;
cleanupProtocol:
	HRSPClientCleanup(&protocolData);
closeSocket:
	closesocket(clientSocket);
cleanupSocket:
	WSACleanup();
	return codeExit;
}
