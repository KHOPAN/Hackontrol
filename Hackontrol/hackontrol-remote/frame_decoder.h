#pragma once

#include "thread_client.h"

void DecodeHRSPFrame(const BYTE* data, size_t size, PSTREAMDATA stream, HWND window);
