#pragma once

#include <jni.h>

jint NativeLibrary_sleep(JNIEnv*, jclass);
jint NativeLibrary_hibernate(JNIEnv*, jclass);
jint NativeLibrary_restart(JNIEnv*, jclass);
jint NativeLibrary_shutdown(JNIEnv*, jclass);
