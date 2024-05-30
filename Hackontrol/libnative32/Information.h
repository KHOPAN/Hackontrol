#pragma once

#include <jni.h>

void InformationRegisterNatives(JNIEnv* const environment);
jstring Information_getUserName(JNIEnv* const environment, jclass const class);
jstring Information_getMachineName(JNIEnv* const environment, jclass const class);
