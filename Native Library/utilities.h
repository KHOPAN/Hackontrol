#pragma once

#include <Windows.h>
#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif
BOOL System_out_println(JNIEnv*, const char*);
BOOL System_err_println(JNIEnv*, const char*);
#ifdef __cplusplus
}
#endif
