#pragma once

#include <taskschd.h>

#ifdef __cplusplus
extern "C" {
#endif
void HI_FormatError(unsigned long, const char*);
void HI_InitializeComAPI();
wchar_t* HI_GetSystemDirectory(const wchar_t*);
#ifdef __cplusplus
}
#endif
