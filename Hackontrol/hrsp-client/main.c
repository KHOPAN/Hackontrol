#include <WS2tcpip.h>
#include "hrsp_client.h"

#define DEFAULT_PORT L"42485"

#define KEY_LENGTH_AES 32

#define SEND_ERROR(error) status=error;byte=error;send(clientSocket,&byte,1,0)

#pragma warning(disable: 6385)

HRSPCLIENTSTATUS HRSPClientConnect(const PHRPSCLIENTPARAMETER parameter) {
	if(!parameter) {
		return HRSP_CLIENT_NULL_PARAMETER;
	}

	if(!parameter->wsaInitialized) {
		WSADATA data;

		if(WSAStartup(MAKEWORD(2, 2), &data)) {
			return HRSP_CLIENT_WSA_INITIALIZATION_FAILED;
		}
	}

	ADDRINFOW hints = {0};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	HRSPCLIENTSTATUS status = HRSP_CLIENT_OK;

	if(GetAddrInfoW(parameter->address ? parameter->address : L"localhost", parameter->port ? parameter->port : DEFAULT_PORT, &hints, &hints.ai_next)) {
		status = HRSP_CLIENT_HOST_TRANSLATION_FAILED;
		goto cleanupSocket;
	}

	SOCKET clientSocket = INVALID_SOCKET;

	for(PADDRINFOW pointer = hints.ai_next; pointer != NULL; pointer = pointer->ai_next) {
		clientSocket = socket(pointer->ai_family, pointer->ai_socktype, pointer->ai_protocol);

		if(clientSocket == INVALID_SOCKET) {
			FreeAddrInfoW(hints.ai_next);
			status = HRSP_CLIENT_CONNECTION_FAILED;
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
		status = HRSP_CLIENT_CONNECTION_FAILED;
		goto cleanupSocket;
	}

	if(send(clientSocket, "HRSP", 4, 0) == SOCKET_ERROR) {
		status = HRSP_CLIENT_SEND_FAILED;
		goto closeSocket;
	}

	BYTE byte;

	if(recv(clientSocket, &byte, 1, MSG_WAITALL) == SOCKET_ERROR) {
		SEND_ERROR(HRSP_CLIENT_RECEIVE_FAILED);
		goto closeSocket;
	}

	if(!byte) {
		status = HRSP_CLIENT_SERVER_ERROR;
		goto closeSocket;
	}

	BCRYPT_ALG_HANDLE symmetricAlgorithm;

	if(!BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&symmetricAlgorithm, BCRYPT_AES_ALGORITHM, NULL, 0))) {
		SEND_ERROR(HRPS_CLIENT_BCRYPT_CANNOT_OPEN_ALGORITHM);
		goto closeSocket;
	}

	PBYTE aesBytes = KHOPAN_ALLOCATE(KEY_LENGTH_AES);

	if(!aesBytes) {
		SEND_ERROR(HRSP_CLIENT_MEMORY_ALLOCATION_FAILED);
		goto closeSymmetricAlgorithm;
	}

	if(!BCRYPT_SUCCESS(BCryptGenRandom(NULL, aesBytes, KEY_LENGTH_AES, BCRYPT_USE_SYSTEM_PREFERRED_RNG))) {
		SEND_ERROR(HRSP_CLIENT_ERROR_GENERATING_RANDOM_KEY_DATA);
		KHOPAN_DEALLOCATE(aesBytes);
		goto closeSymmetricAlgorithm;
	}

	BCRYPT_KEY_HANDLE symmetricKey;

	if(!BCRYPT_SUCCESS(BCryptGenerateSymmetricKey(symmetricAlgorithm, &symmetricKey, NULL, 0, aesBytes, HRSP_AES_KEY_LENGTH, 0))) {
		SEND_ERROR(HRSP_CLIENT_ERROR_GENERATING_KEY);
		KHOPAN_DEALLOCATE(aesBytes);
		goto closeSymmetricAlgorithm;
	}

	KHOPAN_DEALLOCATE(aesBytes);
	printf("Hello, world!\n");
closeSymmetricAlgorithm:
	BCryptCloseAlgorithmProvider(symmetricAlgorithm, 0);
closeSocket:
	closesocket(clientSocket);
cleanupSocket:
	if(!parameter->wsaInitialized && !parameter->wsaNoCleanup) {
		if(WSACleanup() == SOCKET_ERROR && status == HRSP_CLIENT_OK) {
			status = HRSP_CLIENT_WSA_CLEANUP_FAILED;
		}
	}

	return status;
}

/*#include <WS2tcpip.h>
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
	BOOLEAN activate;
	SOCKET socket;
	PHRSPDATA data;
} BACKGROUNDTHREAD, *PBACKGROUNDTHREAD;

static LRESULT CALLBACK backgroundProcedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {	
	PBACKGROUNDTHREAD background;

	if(message == WM_CREATE) {
		SetWindowLongPtrW(window, GWLP_USERDATA, (LONG_PTR) (background = (PBACKGROUNDTHREAD) ((CREATESTRUCT*) lparam)->lpCreateParams));
		return background ? 0 : -1;
	} else if(!(background = (PBACKGROUNDTHREAD) GetWindowLongPtrW(window, GWLP_USERDATA))) {
		return DefWindowProcW(window, message, wparam, lparam);
	}

	switch(message) {
	case WM_DEVICECHANGE:
		if(background->activate) {
			StreamRequestDevice(background->socket, background->data);
		}

		return TRUE;
	}

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

	background->window = CreateWindowExW(0, BACKGROUND_WINDOW_CLASS, NULL, 0, 0, 0, 0, 0, NULL, NULL, instance, background);

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

	((PBACKGROUNDTHREAD) buffer)->socket = clientSocket;
	((PBACKGROUNDTHREAD) buffer)->data = &protocolData;
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
		case HRSP_REMOTE_SERVER_ACTIVATE:
			if(packet.size < 1) break;
			((PBACKGROUNDTHREAD) buffer)->activate = ((PBYTE) packet.data)[0] ? TRUE : FALSE;
			break;
		case HRSP_REMOTE_SERVER_STREAM_DEVICES:
			StreamRequestDevice(clientSocket, &protocolData);
			break;
		case HRSP_REMOTE_SERVER_STREAM_ACTION:
			StreamProcessAction(&packet);
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
}*/
