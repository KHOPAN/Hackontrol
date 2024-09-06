#pragma once

#include <WinSock2.h>

typedef struct {
	WINDOWPLACEMENT placement;
	LONG_PTR style;
} WINDOWSTORAGE;

typedef enum {
	SEND_METHOD_FULL = 0,
	SEND_METHOD_BOUNDARY,
	SEND_METHOD_COLOR,
	SEND_METHOD_UNCOMPRESSED,
} SENDMETHOD;

typedef struct {
	BOOL stream;
	SENDMETHOD method;
	BOOL alwaysOnTop;
	BOOL fullscreen;
	BOOL matchAspectRatio;
	BOOL pictureInPicture;
	BOOL lockFrame;
	BOOL limitToScreen;
} WINDOWCONTEXTMENU;

typedef struct {
	int resizeActivationDistance;
	RECT bounds;
	POINT position;
	BOOL cursorNorth;
	BOOL cursorEast;
	BOOL cursorSouth;
	BOOL cursorWest;
	PBYTE pixels;
	int imageX;
	int imageY;
	int imageWidth;
	int imageHeight;
	int originalImageWidth;
	int originalImageHeight;
} STREAMDATA;

typedef struct {
	HANDLE lock;
	HANDLE thread;
	HWND window;
	WINDOWSTORAGE storage;
	WINDOWCONTEXTMENU menu;
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
