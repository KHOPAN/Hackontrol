#pragma once

#include <Windows.h>

DWORD WINAPI HRSPClientStreamThread(_In_ LPVOID parameter);
BOOL HRSPClientEncodeCurrentFrame(const int width, const int height, const PBYTE screenshotBuffer, const PBYTE qoiBuffer, const PBYTE previousBuffer);
