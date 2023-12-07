#pragma once

#include <string>
#include <jni.h>

void StringUtils_FormatAndSendError(JNIEnv*, jobject, unsigned long);
bool StringUtils_GetClassName(JNIEnv*, jclass, std::string&);
