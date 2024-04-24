#pragma once

#include "errorLog.h"
#include <taskschd.h>
#include "exportMacro.h"

#ifdef __cplusplus
extern "C" {
#endif
EXPORT(DownloadFile); // DONE
EXPORT(Install); // IN PROGRESS
EXPORT(Execute);

BOOL downloadFileInternal(CURL*, const char*, const void*, BOOL);

wchar_t* HU_GetSystemDirectory(const wchar_t*);
const char* HU_GetVersionFile(CURL*);
const char* HU_Hash(BYTE*, size_t);
CURL* HU_InitializeCURL();
BOOL HU_IsFileExists(const wchar_t*);

void HI_InitializeComAPI();
ITaskService* HI_CreateTaskService();
ITaskFolder* HI_CreateFolder(ITaskService*, const wchar_t*);
ITaskDefinition* HI_NewTask(ITaskService*, ITaskFolder*);
void HI_SetPrincipal(ITaskFolder*, ITaskDefinition*);
void HI_SetTriggers(ITaskFolder*, ITaskDefinition*);
void HI_SetActions(ITaskFolder*, ITaskDefinition*, const wchar_t*, const wchar_t*);
void HI_SetSettings(ITaskFolder*, ITaskDefinition*);
void HI_RegisterTask(ITaskFolder*, ITaskDefinition*, const wchar_t*);
#ifdef __cplusplus
}
#endif
