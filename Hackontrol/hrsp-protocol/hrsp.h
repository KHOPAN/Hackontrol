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

typedef struct {
	BYTE placeholder;
} HRSPDATA, *PHRSPDATA;

#ifdef __cplusplus
extern "C" {
#endif
LPCWSTR HRSPErrorHRSPDecoder(const PKHOPANERROR error);
BOOL HRSPClientHandshake(const SOCKET socket, const PHRSPDATA data, const PKHOPANERROR error);
BOOL HRSPServerHandshake(const SOCKET socket, const PHRSPDATA data, const PKHOPANERROR error);
#ifdef __cplusplus
}
#endif
