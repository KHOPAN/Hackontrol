#pragma once

#include <Windows.h>

BOOL MainWindowInitialize(const HINSTANCE instance);
int MainWindowMessageLoop();
void MainWindowRefreshListView();
void MainWindowExit();
