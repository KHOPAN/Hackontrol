#pragma once

#include "libkhopanlist.h"

#define CURL_STATICLIB
#include <curl/curl.h>

#ifdef __cplusplus
extern "C" {
#endif
BOOL KHOPANDownloadData(const PDATASTREAM stream, const LPCSTR address, const BOOL initialized, const BOOL force, const PKHOPANERROR error);
#ifdef __cplusplus
}
#endif
