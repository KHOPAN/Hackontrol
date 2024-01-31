#pragma once

#include <Windows.h>

#define EXPORT __declspec(dllexport) void __stdcall

#ifdef __cplusplus
extern "C" {
#endif
EXPORT Execute(HWND, HINSTANCE, LPSTR, int);
BYTE* screenshot(size_t*);
#ifdef __cplusplus
}
#endif
