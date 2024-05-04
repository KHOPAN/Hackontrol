#pragma once

#include <jni.h>

jstring NativeLibrary_sleep(JNIEnv* environment, jclass nativeLibraryClass);
jstring NativeLibrary_hibernate(JNIEnv* environment, jclass nativeLibraryClass);
jstring NativeLibrary_restart(JNIEnv* environment, jclass nativeLibraryClass);
jstring NativeLibrary_shutdown(JNIEnv* environment, jclass nativeLibraryClass);
