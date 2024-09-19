#pragma once

#include <Windows.h>
#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif
void KHOPANJavaThrow(JNIEnv* environment, const LPCSTR class, const LPCWSTR message);
#ifdef __cplusplus
}
#endif
