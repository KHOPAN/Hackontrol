#pragma once

#include "hrsp.h"

typedef struct {
	int size;
	UINT type;
	PBYTE data;
} HRSPPACKET, *PHRSPPACKET;

#ifdef __cplusplus
extern "C" {
#endif
BOOL HRSPSendPacket(const SOCKET socket, const PHRSPDATA data, const PHRSPPACKET packet, const PHRSPERROR error);
BOOL HRSPReceivePacket(const SOCKET socket, const PHRSPDATA data, const PHRSPPACKET packet, const PHRSPERROR error);
BOOL HRSPFreePacket(const PHRSPPACKET packet, const PHRSPERROR error);
BOOL HRSPSendTypePacket(const SOCKET socket, const PHRSPDATA data, const UINT type, const PHRSPERROR error);
#ifdef __cplusplus
}
#endif
