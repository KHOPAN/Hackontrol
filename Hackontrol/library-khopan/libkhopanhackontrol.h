#pragma once

#include <Windows.h>

#define FILE_LIBDLL32 L"libdll32.dll"

#ifdef __cplusplus
extern "C" {
#endif
LPWSTR KHOPANHackontrolGetHomeDirectory();
#ifdef __cplusplus
}
#endif
