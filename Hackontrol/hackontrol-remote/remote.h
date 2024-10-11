#pragma once

#include <libkhopan.h>
#include "hrsp.h"

#define LOGGER_ENABLE
//#define NO_CONSOLE

#define REMOTE_PORT             L"42485"
#define CLASS_HACKONTROL_REMOTE L"HackontrolRemote"
#define CLASS_REMOTE_SESSION    L"HackontrolRemoteSession"
#define CLASS_SESSION_STREAM    L"HackontrolRemoteSessionStream"

#ifdef LOGGER_ENABLE
#ifdef NO_CONSOLE
#define LOG(format, ...) do{LPSTR __format_message__=KHOPANFormatANSI(format,__VA_ARGS__);if(__format_message__){OutputDebugStringA(__format_message__);LocalFree(__format_message__);}}while(0)
#else
#define LOG(format, ...) do{printf(format,__VA_ARGS__);_flushall();}while(0)
#endif
#else
#define LOG(format, ...)
#endif

typedef struct {
	LPWSTR name;
	WCHAR address[16];
	SOCKET socket;
	HANDLE thread;
	HRSPDATA hrsp;
	HANDLE mutex;

	struct {
		HANDLE thread;
		HWND window;

		struct {
			HANDLE thread;
			HWND window;
			int sourceWidth;
			int sourceHeight;
			int imageWidth;
			int imageHeight;
			int imageX;
			int imageY;
			PBYTE pixels;
		} stream;
	} session;
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
BOOL WindowStreamInitialize();
DWORD WINAPI WindowStream(_In_ PCLIENT client);
void WindowStreamFrame(const PCLIENT client, const PBYTE data, const size_t size);
void WindowStreamClose(const PCLIENT client);
