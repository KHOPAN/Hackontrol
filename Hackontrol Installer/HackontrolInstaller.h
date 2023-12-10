#pragma once

#include <taskschd.h>

#ifdef __cplusplus
extern "C" {
#endif
void HI_FormatError(unsigned long, const char*);
void HI_InitializeComAPI();
ITaskService* HI_CreateTaskService();
ITaskFolder* HI_CreateFolder(ITaskService*, const wchar_t*);
ITaskDefinition* HI_NewTask(ITaskService*, ITaskFolder*);
void HI_SetPrincipal(ITaskFolder*, ITaskDefinition*);
void HI_SetTriggers(ITaskFolder*, ITaskDefinition*);
void HI_SetActions(ITaskFolder*, ITaskDefinition*, const wchar_t*);
void HI_SetSettings(ITaskFolder*, ITaskDefinition*);
void HI_RegisterTask(ITaskFolder*, ITaskDefinition*, const wchar_t*);
wchar_t* HI_GetSystemDirectory(const wchar_t*);
#ifdef __cplusplus
}
#endif
