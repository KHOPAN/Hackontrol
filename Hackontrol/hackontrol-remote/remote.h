#pragma once

#include <libkhopanlist.h>
#include <hrsp.h>

#define LOGGER_ENABLE

#ifdef LOGGER_ENABLE
#define LOG(format, ...) do{printf(format,__VA_ARGS__);_flushall();}while(0)
#else
#define LOG(format, ...)
#endif

#define USERDATA(type, name, window, message, wparam, lparam) type name=NULL;if(message==WM_CREATE){name=(type)(((CREATESTRUCT*)lparam)->lpCreateParams);SetWindowLongPtrW(window,GWLP_USERDATA,(LONG_PTR)(name));}else{name=(type)(GetWindowLongPtrW(window,GWLP_USERDATA));}if(!(name))return DefWindowProcW(window,message,wparam,lparam)

typedef struct {
	WCHAR address[16];
	SOCKET socket;
	HANDLE thread;
	HRSPDATA hrsp;
	LPWSTR name;

	struct {
		HANDLE thread;
		HWND window;
		HWND tab;
		HWND select;

		struct TABLIST {
			HWND tab;
			ULONGLONG data;
		} *tabs;
	} session;
} CLIENT, *PCLIENT, **PPCLIENT;

DWORD WINAPI ThreadClient(_In_ PCLIENT client);
void ThreadClientOpen(const PCLIENT client);
void ThreadClientDisconnect(const PCLIENT client);
DWORD WINAPI ThreadServer(_In_ SOCKET* socketListen);
BOOLEAN WindowMainInitialize();
void WindowMain();
BOOLEAN WindowMainAdd(const PPCLIENT client, const PPLINKEDLISTITEM item);
BOOLEAN WindowMainRemove(const PLINKEDLISTITEM item);
void WindowMainExit();
void WindowMainDestroy();
BOOLEAN WindowSessionInitialize();
DWORD WINAPI WindowSession(_In_ PCLIENT client);
BOOLEAN WindowSessionHandlePacket(const PCLIENT client, const PHRSPPACKET packet);
void WindowSessionClose(const PCLIENT client);
void WindowSessionCleanup();
