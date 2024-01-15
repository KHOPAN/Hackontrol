#pragma once

#include <Windows.h>

//#define EXPORT __declspec(dllexport) void __stdcall
#define EXPORT void

#ifdef __cplusplus
extern "C" {
#endif
	EXPORT Execute(HWND, HINSTANCE, LPSTR, int);
#ifdef __cplusplus
}
#endif
