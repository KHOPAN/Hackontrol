#pragma once

#include <jni.h>

void UserRegisterNatives(JNIEnv* const environment);
jfloat User_getMasterVolume(JNIEnv* const environment, const jclass class);
jboolean User_isMute(JNIEnv* const environment, const jclass class);
void User_setMasterVolume(JNIEnv* const environment, const jclass class, const jfloat volume);
void User_setMute(JNIEnv* const environment, const jclass class, const jboolean mute);
jint User_showMessageBox(JNIEnv* const environment, const jclass class, const jstring title, const jstring content, const jint flags);
jbyteArray User_screenshot(JNIEnv* const environment, const jclass class);
