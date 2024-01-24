#pragma once

#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif
BOOL EnablePrivilege(LPCWSTR);
BOOL ProtectProcess();
#ifdef __cplusplus
}
#endif
