#pragma once

#include <WinSock2.h>

typedef enum {
	SEND_METHOD_FULL = 0,
	SEND_METHOD_BOUNDARY,
	SEND_METHOD_COLOR,
	SEND_METHOD_UNCOMPRESSED,
} SENDMETHOD;

typedef struct {
	PBYTE pixels;
	int imageX;
	int imageY;
	int imageWidth;
	int imageHeight;
	int resizeActivationDistance;
	int originalImageWidth;
	int originalImageHeight;
	int pressedX;
	int pressedY;
	int pressedOffsetX;
	int pressedOffsetY;
	int mouseX;
	int mouseY;
	int savedTop;
	int savedBottom;
	BOOL cursorNorth;
	BOOL cursorEast;
	BOOL cursorSouth;
	BOOL cursorWest;
	BOOL streaming;
	SENDMETHOD sendMethod;
	BOOL pictureInPictureMode;
	BOOL lockFrame;
	BOOL limitToScreen;
} STREAMDATA;

typedef struct {
	HANDLE lock;
	HANDLE thread;
	HWND window;
	STREAMDATA stream;
} WINDOWDATA, *PWINDOWDATA;

typedef struct {
	BOOL active;
	HANDLE thread;
	SOCKET socket;
	LPWSTR name;
	WCHAR address[16];
	PWINDOWDATA window;
} CLIENT, *PCLIENT;

DWORD WINAPI ClientThread(_In_ PCLIENT client);
void ClientOpen(const PCLIENT client);
void ClientDisconnect(const PCLIENT client);
