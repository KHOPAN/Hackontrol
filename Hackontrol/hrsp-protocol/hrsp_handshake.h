#pragma once

#include "hrsp.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL HRSPClientHandshake(const SOCKET socket, const PHRSPPROTOCOLDATA data, const PHRSPPROTOCOLERROR error);
BOOL HRSPServerHandshake(const SOCKET socket, const PHRSPPROTOCOLDATA data, const PHRSPPROTOCOLERROR error);

#ifdef __cplusplus
}
#endif
