#pragma once

#include <Windows.h>

BOOL ExtractJRE(LPCSTR const path);
BOOL CreateUIAccessToken(const LPHANDLE token);
HINSTANCE GetProgramInstance();
