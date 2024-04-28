#pragma once

#include <jni.h>

unsigned int KHStandardOutput(JNIEnv*, const char*);
unsigned int KHStandardError(JNIEnv*, const char*);
unsigned int KHWin32Error(JNIEnv*, unsigned long, const char*);
