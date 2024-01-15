#pragma once

#include <Windows.h>

//#define EXPORT __declspec(dllexport) void __stdcall
#define EXPORT void

#ifdef __cplusplus
extern "C" {
#endif
	EXPORT Execute(HWND, HINSTANCE, LPSTR, int);
	unsigned int lodepng_encode32_noerror(unsigned char**, size_t*, const unsigned char*, unsigned int, unsigned int);
	BYTE* screenshot(size_t*);
#ifdef __cplusplus
}
#endif
