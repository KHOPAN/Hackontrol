#pragma once

#include <Windows.h>

BOOL DecodeHRSPFrame(const BYTE* data, size_t size, HBITMAP* output);
