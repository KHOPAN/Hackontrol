#pragma once

#include "errorLog.h"
#include <taskschd.h>
#include "exportMacro.h"

EXPORT(DownloadFile); // DONE
EXPORT(Install); // DONE
EXPORT(Execute); // IN PROGRESS

char* hashSHA512(BYTE*, size_t);
