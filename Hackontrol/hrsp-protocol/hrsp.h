#pragma once

#include <libkhopan.h>

#define HRSP_PROTOCOL_VERSION       1
#define HRSP_PROTOCOL_VERSION_MINOR 0
#define HRSP_PROTOCOL_PORT          42485
#define HRSP_PROTOCOL_PORT_STRING   L"42485"

#define ERROR_FACILITY_HRSP 0x0005

#undef KHOPAN_ERROR_DECODER
#define KHOPAN_ERROR_DECODER HRSPErrorHRSPDecoder

enum ERRORFACLIITYHRSP {
	ERROR_HRSP_INVALID_MAGIC,
	ERROR_HRSP_UNSUPPORTED_VERSION,
	ERROR_HRSP_CONNECTION_CLOSED
};

typedef unsigned long long HRSPSERVERDATA, *PHRSPSERVERDATA;
typedef unsigned long long HRSPDATA, *PHRSPDATA;

typedef struct {
	UINT type;
	size_t size;
	LPVOID data;
} HRSPPACKET, *PHRSPPACKET;

#ifdef __cplusplus
extern "C" {
#endif
BOOL HRSPClientInitialize(const SOCKET socket, const PHRSPDATA data, const PKHOPANERROR error);
void HRSPClientCleanup(const PHRSPDATA data);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
BOOL HRSPServerInitialize(const PHRSPSERVERDATA server, const PKHOPANERROR error);
BOOL HRSPServerSessionInitialize(const SOCKET socket, const PHRSPDATA data, const PHRSPSERVERDATA server, const PKHOPANERROR error);
void HRSPServerSessionCleanup(const PHRSPDATA data);
void HRSPServerCleanup(const PHRSPSERVERDATA server);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
LPCWSTR HRSPErrorHRSPDecoder(const PKHOPANERROR error);
BOOL HRSPPacketSend(const PHRSPDATA data, const PHRSPPACKET packet, const PKHOPANERROR error);
BOOL HRSPPacketReceive(const PHRSPDATA data, const PHRSPPACKET packet, const PKHOPANERROR error);
#ifdef __cplusplus
}
#endif
