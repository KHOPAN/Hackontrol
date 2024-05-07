#pragma once

#include <Windows.h>
#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif
void KHJavaStandardOutputA(JNIEnv* environment, const LPSTR message);
void KHJavaStandardOutputW(JNIEnv* environment, const LPWSTR message);
void KHJavaStandardErrorA(JNIEnv* environment, const LPSTR message);
void KHJavaStandardErrorW(JNIEnv* environment, const LPWSTR message);
void KHJavaWin32ErrorA(JNIEnv* environment, DWORD errorCode, const LPSTR functionName);
void KHJavaWin32ErrorW(JNIEnv* environment, DWORD errorCode, const LPWSTR functionName);
LPSTR KHJavaToNativeStringA(JNIEnv* environment, jstring string);
LPWSTR KHJavaToNativeStringW(JNIEnv* environment, jstring string);
#ifdef __cplusplus
}
#endif

#ifdef UNICODE
#define KHJavaStandardOutput KHJavaStandardOutputW
#define KHJavaStandardError  KHJavaStandardErrorW
#define KHJavaWin32Error     KHJavaWin32ErrorW
#define KHJavaToNativeString KHJavaToNativeStringW
#else
#define KHJavaStandardOutput KHJavaStandardOutputA
#define KHJavaStandardError  KHJavaStandardErrorA
#define KHJavaWin32Error     KHJavaWin32ErrorA
#define KHJavaToNativeString KHJavaToNativeStringA
#endif
