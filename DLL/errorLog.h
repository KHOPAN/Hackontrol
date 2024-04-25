#pragma once

#include <Windows.h>
#include "curlLib.h"

void curlError(CURLcode, const wchar_t*);
void dialogError(DWORD, const wchar_t*);
void HU_DisplayError(DWORD, const wchar_t*);
