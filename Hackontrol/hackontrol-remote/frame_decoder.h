#pragma once

#include "thread_client.h"

BOOL DecodeHRSPFrame(const BYTE* data, size_t size, STREAMDATA* stream);
