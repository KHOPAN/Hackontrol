#pragma once

#include <jni.h>

jstring NativeLibrary_machineName(JNIEnv* environment, jclass nativeLibraryClass);
jstring NativeLibrary_sleep(JNIEnv* environment, jclass nativeLibraryClass);
jstring NativeLibrary_hibernate(JNIEnv* environment, jclass nativeLibraryClass);
jstring NativeLibrary_restart(JNIEnv* environment, jclass nativeLibraryClass);
jstring NativeLibrary_shutdown(JNIEnv* environment, jclass nativeLibraryClass);
jboolean NativeLibrary_critical(JNIEnv* environment, jclass nativeLibraryClass, jboolean critical);
jint NativeLibrary_dialog(JNIEnv* environment, jclass nativeLibraryClass, jstring title, jstring content, jint flags);
jobjectArray NativeLibrary_listProcess(JNIEnv* environment, jclass nativeLibraryClass);
jint Native_currentIdentifier(JNIEnv* environment, jclass nativeLibraryClass);
jboolean Native_terminate(JNIEnv* environment, jclass nativeLibraryClass, jint identifier);
jboolean Native_hasUIAccess(JNIEnv* environment, jclass nativeLibraryClass);
jboolean Native_setVolume(JNIEnv* environment, jclass nativeLibraryClass, jfloat volume);
jfloat Native_getVolume(JNIEnv* environment, jclass nativeLibraryClass);
jboolean Native_setMute(JNIEnv* environment, jclass nativeLibraryClass, jboolean mute);
jboolean Native_getMute(JNIEnv* environment, jclass nativeLibraryClass);
void Native_freeze(JNIEnv* environment, jclass nativeLibraryClass, jboolean freeze);
