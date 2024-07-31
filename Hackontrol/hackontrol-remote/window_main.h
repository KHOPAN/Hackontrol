#pragma once

#include <Windows.h>

#define CLASS_HACKONTROL_REMOTE L"HackontrolRemote"
#define CLASS_CLIENT_WINDOW     L"HackontrolRemoteClientWindow"

BOOL InitializeMainWindow(const HINSTANCE instance);
int MainWindowMessageLoop();
void RefreshMainWindowListView();
void ExitMainWindow();
