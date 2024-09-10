#pragma once

#include "hrsp.h"

typedef struct {
	int placeholder;
} HRSPPROTOCOLPACKET, *PHRSPPROTOCOLPACKET;

#ifdef __cplusplus
extern "C" {
#endif

BOOL HRSPSendPacket(const SOCKET socket, const PHRSPPROTOCOLDATA data, const PHRSPPROTOCOLPACKET packet, const PHRSPPROTOCOLERROR error);
BOOL HRSPReceivePacket(const SOCKET socket, const PHRSPPROTOCOLDATA data, const PHRSPPROTOCOLPACKET packet, const PHRSPPROTOCOLERROR error);

#ifdef __cplusplus
}
#endif
