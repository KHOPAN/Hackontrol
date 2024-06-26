#pragma once

#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif
LPSTR KHFormatMessageA(const LPSTR format, ...);
LPWSTR KHFormatMessageW(const LPWSTR format, ...);
#ifdef __cplusplus
}
#endif

#ifdef UNICODE
#define KHFormatMessage KHFormatMessageW
#else
#define KHFormatMessage KHFormatMessageA
#endif
