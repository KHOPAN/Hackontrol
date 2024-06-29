#pragma once

#include <khopancurl.h>
#include "hackontrol.h"

#define FORCE_DOWNLOAD_DELAY 1000

BOOL HackontrolDownloadData(DataStream* const stream, LPCSTR const url, const BOOL curlInitialized, CURLcode* const outputCode);
BOOL HackontrolForceDownload(DataStream* const stream, LPCSTR const url, const BOOL curlInitialized);
