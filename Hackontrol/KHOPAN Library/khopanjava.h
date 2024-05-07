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
jstring KHJavaFromNativeStringA(JNIEnv* environment, const LPSTR string);
jstring KHJavaFromNativeStringW(JNIEnv* environment, const LPWSTR string);
void KHJavaThrowA(JNIEnv* environment, const LPSTR throwableClass, const LPSTR message);
void KHJavaThrowW(JNIEnv* environment, const LPSTR throwableClass, const LPWSTR message);
void KHJavaThrowRuntimeExceptionA(JNIEnv* environment, const LPSTR message);
void KHJavaThrowRuntimeExceptionW(JNIEnv* environment, const LPWSTR message);
void KHJavaThrowIllegalArgumentExceptionA(JNIEnv* environment, const LPSTR message);
void KHJavaThrowIllegalArgumentExceptionW(JNIEnv* environment, const LPWSTR message);
void KHJavaThrowInternalErrorA(JNIEnv* environment, const LPSTR message);
void KHJavaThrowInternalErrorW(JNIEnv* environment, const LPWSTR message);
#ifdef __cplusplus
}
#endif

#ifdef UNICODE
#define KHJavaStandardOutput                KHJavaStandardOutputW
#define KHJavaStandardError                 KHJavaStandardErrorW
#define KHJavaWin32Error                    KHJavaWin32ErrorW
#define KHJavaToNativeString                KHJavaToNativeStringW
#define KHJavaFromNativeString              KHJavaFromNativeStringW
#define KHJavaThrow                         KHJavaThrowW
#define KHJavaThrowRuntimeException         KHJavaThrowRuntimeExceptionW
#define KHJavaThrowIllegalArgumentException KHJavaThrowIllegalArgumentExceptionW
#define KHJavaThrowInternalError            KHJavaThrowInternalErrorW
#else
#define KHJavaStandardOutput                KHJavaStandardOutputA
#define KHJavaStandardError                 KHJavaStandardErrorA
#define KHJavaWin32Error                    KHJavaWin32ErrorA
#define KHJavaToNativeString                KHJavaToNativeStringA
#define KHJavaFromNativeString              KHJavaFromNativeStringA
#define KHJavaThrow                         KHJavaThrowA
#define KHJavaThrowRuntimeException         KHJavaThrowRuntimeExceptionA
#define KHJavaThrowIllegalArgumentException KHJavaThrowIllegalArgumentExceptionA
#define KHJavaThrowInternalError            KHJavaThrowInternalErrorA
#endif
