#pragma once

#include <Windows.h>

#define CLASS_HACKONTROL_REMOTE L"HackontrolRemote"

BOOL InitializeMainWindow(const HINSTANCE instance);
int MainWindowMessageLoop();
