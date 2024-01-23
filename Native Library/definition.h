#pragma once

#include <Windows.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
bool EnablePrivilege(LPCWSTR);
bool ProtectProcess();
#ifdef __cplusplus
}
#endif
