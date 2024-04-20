#pragma once

#include <stdio.h>
#include <Windows.h>

void consoleError(unsigned long, const wchar_t*);
wchar_t* getSystem32Directory();
wchar_t* mergePath(const wchar_t*, const wchar_t*);
