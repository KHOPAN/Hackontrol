#pragma once

#include "errorLog.h"
#include <taskschd.h>
#include "exportMacro.h"

EXPORT(DownloadFile); // DONE
EXPORT(Install); // DONE
EXPORT(Execute); // IN PROGRESS

BOOL downloadFileInternal(CURL*, const char*, const void*, BOOL);

char* hashSHA512(BYTE*, size_t);
BOOL HU_IsFileExists(const wchar_t*);
