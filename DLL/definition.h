#pragma once

#include "errorLog.h"
#include <taskschd.h>
#include "exportMacro.h"

#ifdef __cplusplus
extern "C" {
#endif
EXPORT(DownloadFile); // DONE
EXPORT(Install); // DONE
EXPORT(Execute);

BOOL downloadFileInternal(CURL*, const char*, const void*, BOOL);

const char* HU_Hash(BYTE*, size_t);
BOOL HU_IsFileExists(const wchar_t*);
#ifdef __cplusplus
}
#endif
