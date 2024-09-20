#pragma once

//#include <khopancurl.h>
//#include "hackontrol.h"
//#include <Windows.h>
#include <libkhopanlist.h>
#include <libkhopancurl.h>

#define DELAY_FORCE_DOWNLOAD 1000

CURLcode HackontrolDownload(const LPCSTR location, const PDATASTREAM stream, const BOOL initialized, const BOOL force);
//BOOL HackontrolDownloadData(DataStream* const stream, LPCSTR const url, const BOOL curlInitialized, CURLcode* const outputCode);
//BOOL HackontrolForceDownload(DataStream* const stream, LPCSTR const url, const BOOL curlInitialized);
