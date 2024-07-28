#pragma once

#define LOGGER_ENABLE

#ifdef LOGGER_ENABLE
#include <stdio.h>
#define LOG(x) printf(x)
#else
#define LOG(x)
#endif
