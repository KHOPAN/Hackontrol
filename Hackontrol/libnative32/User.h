#pragma once

#include <jni.h>

void UserRegisterNatives(JNIEnv* const environment);
jint User_showMessageBox(JNIEnv* const environment, const jclass class, const jstring title, const jstring content, const jint flags);
