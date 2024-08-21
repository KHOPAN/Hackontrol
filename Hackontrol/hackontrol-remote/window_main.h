#pragma once

#include <Windows.h>

#define CLASS_HACKONTROL_REMOTE L"HackontrolRemote"

BOOL MainWindowInitialize(const HINSTANCE instance);
int MainWindowMessageLoop();
void MainWindowRefreshListView();
void MainWindowExit();
