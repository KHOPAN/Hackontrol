#pragma once

//#define LOGGER_ENABLE
//#define NO_CONSOLE

#ifdef LOGGER_ENABLE
#include <stdio.h>
#define COMMA ,
#ifdef NO_CONSOLE
#include <khopanstring.h>
#include <debugapi.h>
#define LOG(x) do{LPSTR xxx_result_xxx=KHFormatMessageA(x);if(xxx_result_xxx){OutputDebugStringA(xxx_result_xxx);LocalFree(xxx_result_xxx);}}while(0)
#else
#define LOG(x) do{printf(x);_flushall();}while(0)
#endif
#else
#define LOG(x)
#endif
