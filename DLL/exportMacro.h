#pragma once

#define EXPORT(x) __declspec(dllexport) void __stdcall x(HWND window, HINSTANCE instance, LPSTR argument, int command)
