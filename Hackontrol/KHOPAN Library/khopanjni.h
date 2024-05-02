#pragma once

#include <Windows.h>
#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

void KHStandardOutputA(JNIEnv* environment, const LPSTR message);
void KHStandardOutputW(JNIEnv* environment, const LPWSTR message);

void KHStandardErrorA(JNIEnv* environment, const LPSTR message);
void KHStandardErrorW(JNIEnv* environment, const LPWSTR message);

void KHWin32ErrorA(JNIEnv* environment, DWORD errorCode, const LPSTR functionName);
void KHWin32ErrorW(JNIEnv* environment, DWORD errorCode, const LPWSTR functionName);

#ifdef __cplusplus
}
#endif
