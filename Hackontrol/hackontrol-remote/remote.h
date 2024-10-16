#pragma once

#include <libkhopan.h>
#include "hrsp.h"

#define LOGGER_ENABLE
//#define NO_CONSOLE

#define REMOTE_PORT             L"42485"
#define CLASS_HACKONTROL_REMOTE L"HackontrolRemote"
#define CLASS_REMOTE_SESSION    L"HackontrolRemoteSession"

#ifdef LOGGER_ENABLE
#ifdef NO_CONSOLE
#define LOG(format, ...) do{LPSTR __format_message__=KHOPANFormatANSI(format,__VA_ARGS__);if(__format_message__){OutputDebugStringA(__format_message__);LocalFree(__format_message__);}}while(0)
#else
#define LOG(format, ...) do{printf(format,__VA_ARGS__);_flushall();}while(0)
#endif
#else
#define LOG(format, ...)
#endif

#define USERDATA(type, name, window, message, wparam, lparam) type name=NULL;if(message==WM_CREATE){name=(type)(((CREATESTRUCT*)lparam)->lpCreateParams);SetWindowLongPtrW(window,GWLP_USERDATA,(LONG_PTR)(name));}else{name=(type)(GetWindowLongPtrW(window,GWLP_USERDATA));}if(!(name))return DefWindowProcW(window,message,wparam,lparam)
#define SIZEOFARRAY(x) (sizeof(x)/sizeof((x)[0]))

typedef struct {
	HANDLE mutex;
	HANDLE thread;
	HWND window;
	int resizeActivationDistance;
	int sourceWidth;
	int sourceHeight;
	int imageWidth;
	int imageHeight;
	int imageX;
	int imageY;
	PBYTE pixels;

	struct {
		BOOL stream;

		enum {
			SEND_METHOD_FULL = 0,
			SEND_METHOD_BOUNDARY,
			SEND_METHOD_COLOR,
			SEND_METHOD_UNCOMPRESSED,
		} method;

		BOOL fullscreen;
		BOOL pictureInPicture;
		BOOL lockFrame;
		BOOL limitToScreen;
	} menu;

	struct {
		WINDOWPLACEMENT placement;
		LONG_PTR style;
	} fullscreen;

	BOOL cursorNorth;
	BOOL cursorEast;
	BOOL cursorSouth;
	BOOL cursorWest;
	POINT position;
	RECT bounds;
} STREAM;

typedef struct {
	HANDLE thread;
	HWND* tabs;
	HWND window;
	HWND tab;
	HWND selectedTab;
	STREAM stream;
} SESSION;

typedef struct {
	WCHAR address[16];
	SOCKET socket;
	HANDLE thread;
	HRSPDATA hrsp;
	LPWSTR name;
	SESSION session;
} CLIENT, *PCLIENT;

DWORD WINAPI ThreadClient(_In_ PCLIENT client);
void ThreadClientOpen(const PCLIENT client);
void ThreadClientDisconnect(const PCLIENT client);

DWORD WINAPI ThreadServer(_In_ SOCKET* socketListen);

int WindowMain();
void WindowMainRefresh();
void WindowMainExit();

BOOL WindowSessionInitialize();
DWORD WINAPI WindowSession(_In_ PCLIENT client);
void WindowSessionClose(const PCLIENT client);
void WindowSessionCleanup();
