#pragma once

#include <WinSock2.h>

typedef enum {
	SEND_METHOD_FULL = 0,
	SEND_METHOD_BOUNDARY,
	SEND_METHOD_COLOR,
	SEND_METHOD_UNCOMPRESSED,
} SENDMETHOD;

typedef struct {
	BOOL stream;
	SENDMETHOD method;
	BOOL pictureInPicture;
	BOOL lockFrame;
	BOOL limitToScreen;
} STREAMCONTEXTMENU;

typedef struct {
	STREAMCONTEXTMENU contextMenu;
	PBYTE pixels;
	int resizeActivationDistance;
	int imageX;
	int imageY;
	int imageWidth;
	int imageHeight;
	int originalImageWidth;
	int originalImageHeight;
	RECT bounds;
	POINT position;
	BOOL cursorNorth;
	BOOL cursorEast;
	BOOL cursorSouth;
	BOOL cursorWest;
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
