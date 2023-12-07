#pragma once

#include <string>
#include <jni.h>

void ErrorUtils_ThrowException(JNIEnv*, const std::string&, const std::string&);
