#pragma once

#include <Windows.h>

#define EXPORT __declspec(dllexport) void __stdcall

#ifdef __cplusplus
extern "C" {
#endif
	EXPORT Execute(HWND, HINSTANCE, LPSTR, int);
	void HJ_DisplayError(DWORD, const wchar_t*);
	wchar_t* HJ_GetSystemDirectory(const wchar_t*);
#ifdef __cplusplus
}
#endif
