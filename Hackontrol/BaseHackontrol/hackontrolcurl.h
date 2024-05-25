#pragma once

#include <khopancurl.h>
#include "hackontrol.h"

BOOL HackontrolDownloadData(DataStream* const stream, LPCSTR const url, const BOOL curlInitialized, CURLcode* const outputCode);
