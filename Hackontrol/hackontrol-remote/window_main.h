#pragma once

#include <Windows.h>

#define CLASS_HACKONTROL_REMOTE L"HackontrolRemote"

BOOL InitializeMainWindow();
int MainWindowMessageLoop();
void RefreshMainWindowListView();
void ExitMainWindow();
