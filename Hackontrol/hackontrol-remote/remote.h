#pragma once

#include <libkhopan.h>
#include <hrsp.h>
#include <hrsp_packet.h>

#define LOGGER_ENABLE

#ifdef LOGGER_ENABLE
#define LOG(format, ...) do{printf(format,__VA_ARGS__);_flushall();}while(0)
#else
#define LOG(format, ...)
#endif

#define REMOTE_PORT          L"42485"
#define CLASS_REMOTE         L"HackontrolRemote"
#define CLASS_REMOTE_SESSION L"HackontrolRemoteSession"

#define USERDATA(type, name, window, message, wparam, lparam) type name=NULL;if(message==WM_CREATE){name=(type)(((CREATESTRUCT*)lparam)->lpCreateParams);SetWindowLongPtrW(window,GWLP_USERDATA,(LONG_PTR)(name));}else{name=(type)(GetWindowLongPtrW(window,GWLP_USERDATA));}if(!(name))return DefWindowProcW(window,message,wparam,lparam)
#define SIZEOFARRAY(x) (sizeof(x)/sizeof((x)[0]))

typedef struct {
	HWND tab;
	ULONGLONG data;
} TABSTORE, *PTABSTORE;

typedef struct {
	HANDLE thread;
	PTABSTORE tabs;
	HWND window;
	HWND tab;
	HWND selectedTab;
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

BOOL WindowMainInitialize();
void WindowMainDestroy();
void WindowMain();
void WindowMainRefresh();
void WindowMainExit();

BOOL WindowSessionInitialize();
DWORD WINAPI WindowSession(_In_ PCLIENT client);
BOOL WindowSessionHandlePacket(const PCLIENT client, const PHRSPPACKET packet);
void WindowSessionClose(const PCLIENT client);
void WindowSessionCleanup();
