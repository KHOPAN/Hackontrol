#pragma once

#include <Windows.h>

#define EXPORT __declspec(dllexport) void __stdcall

#ifdef __cplusplus
extern "C" {
#endif
	EXPORT Download(HWND, HINSTANCE, LPSTR, int);
#ifdef __cplusplus
}
#endif
