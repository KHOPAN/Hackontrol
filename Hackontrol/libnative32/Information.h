#pragma once

#include <jni.h>

void InformationRegisterNatives(JNIEnv* const environment);
jstring Information_getToken(JNIEnv* const environment, const jclass class);
jstring Information_getUserName(JNIEnv* const environment, const jclass class);
jstring Information_getMachineName(JNIEnv* const environment, const jclass class);
jboolean Information_isEnabledUIAccess(JNIEnv* const environment, const jclass class);
