#pragma once

#include <jni.h>

jstring NativeLibrary_sleep(JNIEnv* environment, jclass nativeLibraryClass);
jstring NativeLibrary_hibernate(JNIEnv* environment, jclass nativeLibraryClass);
jstring NativeLibrary_restart(JNIEnv* environment, jclass nativeLibraryClass);
jstring NativeLibrary_shutdown(JNIEnv* environment, jclass nativeLibraryClass);
jboolean NativeLibrary_critical(JNIEnv* environment, jclass nativeLibraryClass, jboolean critical);
jint NativeLibrary_dialog(JNIEnv* environment, jclass nativeLibraryClass, jstring title, jstring content, jint flags);
jobjectArray NativeLibrary_listProcess(JNIEnv* environment, jclass nativeLibraryClass);
