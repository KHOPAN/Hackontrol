#pragma once

#include <libkhopanlist.h>
#include <libkhopancurl.h>

#define DELAY_FORCE_DOWNLOAD 1000

CURLcode HackontrolDownload(const LPCSTR location, const PDATASTREAM stream, const BOOL initialized, const BOOL force);
