#pragma once

#include <WinSock2.h>

typedef enum {
	SEND_METHOD_FULL         = 0,
	SEND_METHOD_BOUNDARY     = 1,
	SEND_METHOD_COLOR        = 2,
	SEND_METHOD_UNCOMPRESSED = 3
} SENDMETHOD;

typedef struct {
	BOOL streaming;
	SENDMETHOD method;
	BOOL pictureInPicture;
	int width;
	int height;
	int x;
	int y;
	int imageWidth;
	int imageHeight;
	int pressedX;
	int pressedY;
	PBYTE pixels;
	HANDLE lock;
} STREAMDATA, *PSTREAMDATA;

typedef struct {
	BOOL active;
	SOCKET socket;
	WCHAR address[16];
	LPWSTR name;
	HANDLE thread;
	HANDLE windowThread;
	HWND clientWindow;
	PSTREAMDATA stream;
} CLIENT, *PCLIENT;

DWORD WINAPI ClientThread(_In_ PCLIENT client);
void ClientOpen(PCLIENT client);
void ClientDisconnect(PCLIENT client);
