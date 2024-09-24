#pragma once

#include "hrsp.h"

#ifdef __cplusplus
extern "C" {
#endif
BOOL HRSPClientHandshake(const SOCKET socket, const PHRSPDATA data, const PHRSPERROR error);
BOOL HRSPServerHandshake(const SOCKET socket, const PHRSPDATA data, const PHRSPERROR error);
#ifdef __cplusplus
}
#endif
