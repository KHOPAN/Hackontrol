#pragma once

#include <Windows.h>

int EnablePrivilege(LPCWSTR privilege);
void ProtectProcess();
