#pragma once

#include <libkhopan.h>

#define LOGGER_ENABLE
//#define NO_CONSOLE

#define REMOTE_PORT L"42485"

#ifdef LOGGER_ENABLE
#ifdef NO_CONSOLE
#define LOG(format, ...) do{LPSTR __format_message__=KHOPANFormatANSI(format,__VA_ARGS__);if(__format_message__){OutputDebugStringA(__format_message__);LocalFree(__format_message__);}}while(0)
#else
#define LOG(format, ...) do{printf(format,__VA_ARGS__);_flushall();}while(0)
#endif
#else
#define LOG(format, ...)
#endif

DWORD WINAPI ThreadServer(_In_ LPVOID parameter);
typedef struct _CLIENT {
	struct _CLIENT* next;
} CLIENT, *PCLIENT;

typedef struct {
	size_t size;
	PCLIENT client;
	HANDLE mutex;
} CLIENTLIST;
