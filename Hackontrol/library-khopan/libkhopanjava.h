#pragma once

#include <Windows.h>
#include <jni.h>

#define KHOPANTHROW_RUNTIME_EXCEPTION(environment, message)          KHOPANJavaThrow(environment,"java/lang/RuntimeException",message)
#define KHOPANTHROW_ILLEGAL_ARGUMENT_EXCEPTION(environment, message) KHOPANJavaThrow(environment,"java/lang/IllegalArgumentException",message)
#define KHOPANTHROW_INTERNAL_ERROR(environment, message)             KHOPANJavaThrow(environment,"java/lang/InternalError",message)

#ifdef __cplusplus
extern "C" {
#endif
void KHOPANJavaThrow(JNIEnv* environment, const LPCSTR class, const LPCWSTR message);
#ifdef __cplusplus
}
#endif
