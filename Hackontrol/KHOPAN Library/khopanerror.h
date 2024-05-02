#pragma once

#include <wchar.h>

char* KHGetWin32ErrorMessageA(unsigned long, const char*);
wchar_t* KHGetWin32ErrorMessageW(unsigned long, const wchar_t*);
void KHWin32DialogErrorW(unsigned long, const wchar_t*);
void KHWin32DialogErrorA(unsigned long, const char*);
void KHWin32ConsoleErrorW(unsigned long, const wchar_t*);
void KHWin32ConsoleErrorA(unsigned long, const char*);
unsigned long KHDecodeHRESULTError(HRESULT);
