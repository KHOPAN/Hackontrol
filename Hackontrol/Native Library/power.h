#pragma once

#include <jni.h>

jstring NativeLibrary_sleep(JNIEnv*, jclass);
jstring NativeLibrary_hibernate(JNIEnv*, jclass);
jstring NativeLibrary_restart(JNIEnv*, jclass);
jstring NativeLibrary_shutdown(JNIEnv*, jclass);
