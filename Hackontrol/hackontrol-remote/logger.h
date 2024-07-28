#pragma once

#define LOGGER_ENABLE

#ifdef LOGGER_ENABLE
#include <stdio.h>
#define COMMA ,
#define LOG(x) do{printf(x);_flushall();}while(0)
#else
#define LOG(x)
#endif
